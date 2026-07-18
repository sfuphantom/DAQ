#include <Arduino.h>
#include <driver/twai.h>
#include <esp_err.h>
#include "systemConfig.h"

static bool canReady = false;
static uint32_t lastStatusMs = 0;

static void printCanStatus(const char *prefix) {
    twai_status_info_t status = {};
    esp_err_t result = twai_get_status_info(&status);
    if (result != ESP_OK) {
        Serial.printf("%s status unavailable: %s\n", prefix, esp_err_to_name(result));
        return;
    }

    Serial.printf(
        "%s state=%d rx_missed=%lu rx_overrun=%lu tx_failed=%lu arb_lost=%lu bus_error=%lu rx_queue=%lu tx_queue=%lu\n",
        prefix,
        status.state,
        status.rx_missed_count,
        status.rx_overrun_count,
        status.tx_failed_count,
        status.arb_lost_count,
        status.bus_error_count,
        status.msgs_to_rx,
        status.msgs_to_tx);
}

void setup() {
    Serial.begin(BAUD_RATE);
    delay(1000);

    Serial.println("CAN RX test");
    Serial.printf("Listening on TX=%d RX=%d bitrate=500k\n", CAN_TX_PIN, CAN_RX_PIN);

    twai_general_config_t generalConfig = TWAI_GENERAL_CONFIG_DEFAULT(CAN_TX_PIN, CAN_RX_PIN, TWAI_MODE_NORMAL);
    generalConfig.tx_queue_len = 5;
    generalConfig.rx_queue_len = 20;
    generalConfig.alerts_enabled = TWAI_ALERT_ALL;

    twai_timing_config_t timingConfig = TWAI_TIMING_CONFIG_500KBITS();
    twai_filter_config_t filterConfig = TWAI_FILTER_CONFIG_ACCEPT_ALL();

    esp_err_t installResult = twai_driver_install(&generalConfig, &timingConfig, &filterConfig);
    if (installResult != ESP_OK) {
        Serial.printf("CAN driver install failed: %s\n", esp_err_to_name(installResult));
        return;
    }

    esp_err_t startResult = twai_start();
    if (startResult != ESP_OK) {
        Serial.printf("CAN driver start failed: %s\n", esp_err_to_name(startResult));
        twai_driver_uninstall();
        return;
    }

    canReady = true;
    Serial.println("CAN ready; waiting for frames");
    printCanStatus("Initial");
}

void loop() {
    if (!canReady) {
        delay(1000);
        return;
    }

    twai_message_t message = {};
    esp_err_t rxResult = twai_receive(&message, pdMS_TO_TICKS(100));
    if (rxResult != ESP_OK) {
        unsigned long now = millis();
        if (now - lastStatusMs >= 1000) {
            Serial.printf("No CAN frame received: %s\n", esp_err_to_name(rxResult));
            printCanStatus("Status");
            lastStatusMs = now;
        }
        return;
    }

    Serial.printf(
        "RX %s %s ID=0x%03X DLC=%d DATA=",
        message.extd ? "EXT" : "STD",
        message.rtr ? "REMOTE" : "DATA",
        message.identifier,
        message.data_length_code);

    for (int i = 0; i < message.data_length_code; i++) {
        Serial.printf("%02X ", message.data[i]);
    }

    Serial.println();
}
