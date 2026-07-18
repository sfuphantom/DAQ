#include "can.h"
#include "logger.h"
#include "systemConfig.h"
#include <Arduino.h>
#include <esp_err.h>

static bool gCanReady = false;
static uint8_t gConsecutiveTxFailures = 0;
static uint32_t gNextRecoveryAttemptMs = 0;

static const char *canStateName(twai_state_t state) {
// convert a twai state enum to a human-readable string
// return string representation of the state
    switch (state) {
        case TWAI_STATE_STOPPED:
            return "stopped";
        case TWAI_STATE_RUNNING:
            return "running";
        case TWAI_STATE_BUS_OFF:
            return "bus-off";
        case TWAI_STATE_RECOVERING:
            return "recovering";
        default:
            return "unknown";
    }
}

static void logCanStatus() {
// log current can status, including state, queue lengths, error counters, and failure counts
// it retrieves the status info from the twai driver and logs it using the logger
// output goes to esp32 usb serial and optionally to sd log if enabled
// used when can self-reception fails during initialization
// view with: pio device monitor -p <port> -b 115200
    twai_status_info_t status = {};
    esp_err_t result = twai_get_status_info(&status);
    if (result != ESP_OK) {
        Logger::error("CAN status unavailable: %s", esp_err_to_name(result));
        return;
    }

    Logger::error(
        "CAN status: state=%s tx_queue=%u rx_queue=%u tx_err=%u rx_err=%u tx_failed=%u bus_errors=%u",
        canStateName(status.state),
        status.msgs_to_tx,
        status.msgs_to_rx,
        status.tx_error_counter,
        status.rx_error_counter,
        status.tx_failed_count,
        status.bus_error_count);
}

static void shutdownCanDriver() {
    if (gCanReady) {
        twai_stop();
    }

    twai_driver_uninstall();
    gCanReady = false;
}

static bool installAndStartCanDriver() {
    twai_general_config_t general_config = {
        .mode = CAN_NO_ACK_MODE ? TWAI_MODE_NO_ACK : TWAI_MODE_NORMAL,
        .tx_io = CAN_TX_PIN,
        .rx_io = CAN_RX_PIN,
        .clkout_io = TWAI_IO_UNUSED,
        .bus_off_io = TWAI_IO_UNUSED,
        .tx_queue_len = 10,
        .rx_queue_len = 10,
        .alerts_enabled = TWAI_ALERT_ALL,
        .clkout_divider = 0
    };

    twai_timing_config_t timing_config = TWAI_TIMING_CONFIG_500KBITS();
    twai_filter_config_t filter_config = TWAI_FILTER_CONFIG_ACCEPT_ALL();

    esp_err_t installResult = twai_driver_install(&general_config, &timing_config, &filter_config);
    if (installResult != ESP_OK) {
        Logger::error("CAN driver install failed: %s", esp_err_to_name(installResult));
        return false;
    }

    esp_err_t startResult = twai_start();
    if (startResult != ESP_OK) {
        Logger::error("CAN driver start failed: %s", esp_err_to_name(startResult));
        twai_driver_uninstall();
        return false;
    }

    gCanReady = true;
    gConsecutiveTxFailures = 0;
#if ENABLE_STATUS_LOGS
    Logger::notice("CAN Initialized (TWAI, mode=%s)", CAN_NO_ACK_MODE ? "no-ack" : "normal");
#endif
    return true;
}

bool canInit() {
// initialize the can twai driver and run a self-reception test in no-ack mode
    gCanReady = false;
    gConsecutiveTxFailures = 0;
    gNextRecoveryAttemptMs = 0;

    if (!installAndStartCanDriver()) {
        return false;
    }

    if (!CAN_NO_ACK_MODE) {
        return true;
    }

#if ENABLE_STATUS_LOGS
    Logger::notice("Testing CAN self-reception...");
#endif
    // construct the can frame for self-reception test
    twai_message_t msg = {};
    msg.identifier = 0x321;
    msg.data_length_code = 2;
    msg.self = 1; // request self reception so we can validate without another node
    msg.data[0] = 0xAB;
    msg.data[1] = 0xCD;

    esp_err_t txResult = twai_transmit(&msg, pdMS_TO_TICKS(1000));
    if (txResult != ESP_OK) {
        // catches driver not started, transmit queue full, timeout, and invalid controller
        Logger::error("CAN self-reception transmit failed: %s", esp_err_to_name(txResult));
        logCanStatus();
        shutdownCanDriver();
        return false;
    }

    twai_message_t rx = {};

    if (twai_receive(&rx, pdMS_TO_TICKS(1000)) != ESP_OK) {
        // free-rtos ticks to wait for a message
        Logger::error("CAN self-reception failed");
        logCanStatus();
        shutdownCanDriver();
        return false;
    }

    // make sure self-reception returned the exact frame we transmitted
    if (rx.identifier != 0x321 ||
        rx.data_length_code != 2 ||
        rx.data[0] != 0xAB ||
        rx.data[1] != 0xCD) {
        Logger::error(
            "Unexpected CAN frame: ID=%X, DLC=%u, data=[%X %X]",
            rx.identifier,
            rx.data_length_code,
            rx.data[0],
            rx.data[1]
        );
        logCanStatus();
        shutdownCanDriver();
        return false;
    }
#if ENABLE_STATUS_LOGS
        Logger::notice("CAN self-reception OK: ID=%X, data=[%X %X]", rx.identifier, rx.data[0], rx.data[1]);
#endif
    return true;
}

static void stopCanForRecovery() {
// stop the can driver and drop future transmissions until recovery is attempted
    if (gCanReady) {
        twai_stop();
        twai_driver_uninstall();
    }

    gCanReady = false;
    gNextRecoveryAttemptMs = millis() + CAN_RECOVERY_COOLDOWN_MS;
    Logger::error(
        "CAN recovery: stopped TWAI after %u consecutive TX failures; dropping frames for %lu ms",
        static_cast<unsigned>(gConsecutiveTxFailures),
        static_cast<unsigned long>(CAN_RECOVERY_COOLDOWN_MS));
}

static bool ensureCanReady() {
    if (gCanReady) {
        return true;
    }

    uint32_t now = millis();
    if (now < gNextRecoveryAttemptMs) {
#if ENABLE_STATUS_LOGS
        static uint32_t lastDropLogMs = 0;
        if (now - lastDropLogMs >= 1000) {
            Logger::notice("CAN recovery: dropping TX while waiting to restart");
            lastDropLogMs = now;
        }
#endif
        return false;
    }

#if ENABLE_STATUS_LOGS
    Logger::notice("CAN recovery: restarting TWAI");
#endif
    if (!installAndStartCanDriver()) {
        gNextRecoveryAttemptMs = now + CAN_RECOVERY_COOLDOWN_MS;
        return false;
    }

#if ENABLE_STATUS_LOGS
    Logger::notice("CAN recovery: TWAI restarted");
#endif
    return true;
}

static void recordCanTxFailure(uint32_t id, esp_err_t txResult) {
    ++gConsecutiveTxFailures;
    Logger::error(
        "CAN TX failed (ID=%X): %s (consecutive=%u)",
        static_cast<unsigned>(id),
        esp_err_to_name(txResult),
        static_cast<unsigned>(gConsecutiveTxFailures));
    logCanStatus();

    if (gConsecutiveTxFailures >= CAN_TX_FAILURE_RECOVERY_THRESHOLD) {
        stopCanForRecovery();
    }
}

static void recordCanTxSuccess() {
    gConsecutiveTxFailures = 0;
}

void canSendInt16(uint16_t id, int16_t value) {
    if (!ensureCanReady()) {
        return;
    }

    twai_message_t msg = {};
    msg.identifier = id;
    msg.data_length_code = 2;
    msg.data[0] = value & 0xFF;
    msg.data[1] = value >> 8;

    esp_err_t txResult = twai_transmit(&msg, pdMS_TO_TICKS(50));
    if (txResult != ESP_OK) {
        recordCanTxFailure(msg.identifier, txResult);
        return;
    }
    recordCanTxSuccess();

#if ENABLE_STATUS_LOGS
    if (id == static_cast<uint16_t>(CANMessageId::CoolingFault)) {
        Logger::notice("CoolingFault CAN TX OK -> %d", value);
    }
    else if (id == static_cast<uint16_t>(CANMessageId::WheelSpeed)) {
        Logger::notice("WheelSpeed CAN TX OK -> %d (centi-kmh)", value);
    }
    else {
        Logger::notice("CAN TX OK -> ID: 0x%X, data: [%X %X]", msg.identifier, msg.data[0], msg.data[1]);
    }
#endif
}

void canSendUInt8(uint16_t id, uint8_t value) {
    if (!ensureCanReady()) {
        return;
    }

    twai_message_t msg = {};
    msg.identifier = id;
    msg.data_length_code = 1;
    msg.data[0] = value;

    esp_err_t txResult = twai_transmit(&msg, pdMS_TO_TICKS(50));
    if (txResult != ESP_OK) {
        recordCanTxFailure(msg.identifier, txResult);
        return;
    }
    recordCanTxSuccess();

#if ENABLE_STATUS_LOGS
    if (id == static_cast<uint16_t>(CANMessageId::CoolingFault)) {
        Logger::notice("CoolingFault CAN TX OK -> %u", value);
    }
    else {
        Logger::notice("CAN TX OK -> ID: 0x%X, data: [%X]", msg.identifier, msg.data[0]);
    }
#endif
}
