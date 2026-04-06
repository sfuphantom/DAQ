#include "can.h"
#include "Logger.h"
#include "system_config.h"
#include <esp_err.h>

void CAN_Init()
{
    twai_general_config_t general_config = {
        .mode = TWAI_MODE_NORMAL,
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

    if (twai_driver_install(&general_config, &timing_config, &filter_config) != ESP_OK ||
        twai_start() != ESP_OK)
    {
        Logger::Error("CAN Init Failed");
        return;
    }

    Logger::Notice("CAN Initialized (TWAI)");

    Logger::Notice("Testing CAN Loopback...");

    twai_message_t msg = {};
    msg.identifier = 0x321;
    msg.data_length_code = 2;
    msg.self = 1; // request self reception so we can validate without another node
    msg.data[0] = 0xAB;
    msg.data[1] = 0xCD;

    twai_transmit(&msg, pdMS_TO_TICKS(1000));

    twai_message_t rx;
    if (twai_receive(&rx, pdMS_TO_TICKS(1000)) == ESP_OK)
    {
        Logger::Notice("LOOPBACK OK — ID: 0x%X, data: [%X %X]", rx.identifier, rx.data[0], rx.data[1]);
    }
    else
    {
        Logger::Error("LOOPBACK FAILED");
    }
}

void CAN_SendInt16(uint16_t id, int16_t value)
{
    twai_message_t msg = {};
    msg.identifier = id;
    msg.data_length_code = 2;
    msg.data[0] = value & 0xFF;
    msg.data[1] = value >> 8;

    if (id == static_cast<uint16_t>(CANMessageId::CoolingFault))
    {
        Logger::Notice("CoolingFault CAN TX -> %d", value);
    }
    else if (id == static_cast<uint16_t>(CANMessageId::WheelSpeed))
    {
        Logger::Notice("WheelSpeed CAN TX -> %d (centi-kmh)", value);
    }
    else
    {
        Logger::Notice("CAN TX -> ID: 0x%X, data: [%X %X]", msg.identifier, msg.data[0], msg.data[1]);
    }

    esp_err_t txResult = twai_transmit(&msg, pdMS_TO_TICKS(50));
    if (txResult != ESP_OK)
    {
        Logger::Error("CAN TX failed (ID: 0x%X): %s", msg.identifier, esp_err_to_name(txResult));
    }
}

void CAN_SendUInt8(uint16_t id, uint8_t value)
{
    twai_message_t msg = {};
    msg.identifier = id;
    msg.data_length_code = 1;
    msg.data[0] = value;

    if (id == static_cast<uint16_t>(CANMessageId::CoolingFault))
    {
        Logger::Notice("CoolingFault CAN TX -> %u", value);
    }
    else
    {
        Logger::Notice("CAN TX -> ID: 0x%X, data: [%X]", msg.identifier, msg.data[0]);
    }

    esp_err_t txResult = twai_transmit(&msg, pdMS_TO_TICKS(50));
    if (txResult != ESP_OK)
    {
        Logger::Error("CAN TX failed (ID: 0x%X): %s", msg.identifier, esp_err_to_name(txResult));
    }
}
