#include "CAN.h"
#include "Logger.h"

void CAN_Init()
{
    can_general_config_t general_config = {
        .mode = CAN_MODE_NO_ACK, //CAN_MODE_LOOPBACK,
        .tx_io = GPIO_NUM_5,
        .rx_io = GPIO_NUM_4,
        .clkout_io = CAN_IO_UNUSED,
        .bus_off_io = CAN_IO_UNUSED,
        .tx_queue_len = 10,
        .rx_queue_len = 10,
        .alerts_enabled = CAN_ALERT_ALL,
        .clkout_divider = 0
    };

    can_timing_config_t timing_config = CAN_TIMING_CONFIG_250KBITS();
    can_filter_config_t filter_config = CAN_FILTER_CONFIG_ACCEPT_ALL();

    if (can_driver_install(&general_config, &timing_config, &filter_config) != ESP_OK ||
        can_start() != ESP_OK)
    {
        Logger::Error("CAN Init Failed");
        return;
    }

    Logger::Notice("CAN Initialized (TWAI)");

    Logger::Notice("Testing CAN Loopback...");

    can_message_t msg = {};
    msg.identifier = 0x321;
    msg.data_length_code = 2;
    msg.data[0] = 0xAB;
    msg.data[1] = 0xCD;

    can_transmit(&msg, pdMS_TO_TICKS(1000));

    can_message_t rx;
    if (can_receive(&rx, pdMS_TO_TICKS(1000)) == ESP_OK)
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
    can_message_t msg = {};
    msg.identifier = id;
    msg.data_length_code = 2;
    msg.data[0] = value & 0xFF;
    msg.data[1] = value >> 8;

    can_transmit(&msg, pdMS_TO_TICKS(50));
}
