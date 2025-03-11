#include "CanDriver.h"
#include "Logger.h"
#include "stdlib.h"
#include "time.h"
#include "stdio.h"

// some of this can be moved to sys_config
void CanDriver::CanInnit()
{
    can_general_config_t general_config = {
        .mode = CAN_MODE_NO_ACK,         // not tested - change to CAN_MODE_NO_ACK if doesn't run
        .tx_io = (gpio_num_t)GPIO_NUM_5, // so far only works on pins 5 and 4
        .rx_io = (gpio_num_t)GPIO_NUM_4,
        .clkout_io = (gpio_num_t)CAN_IO_UNUSED,
        .bus_off_io = (gpio_num_t)CAN_IO_UNUSED,
        .tx_queue_len = 65,
        .rx_queue_len = 65,
        .alerts_enabled = CAN_ALERT_ALL, // TODO: once Dash/BMS/VCU implemented create a filter mask
        .clkout_divider = 0};

    can_timing_config_t timing_config = CAN_TIMING_CONFIG_250KBITS();
    can_filter_config_t filter_config = CAN_FILTER_CONFIG_ACCEPT_ALL();

    esp_err_t error;
    error = can_driver_install(&general_config, &timing_config, &filter_config);

    if (error == ESP_OK)
    {
        Logger::Notice("CAN Driver Installation OK");
    }

    else
    {
        while (1)
        {
            Logger::Error("CAN Driver Instalation Failed");
        }
        return;
    }

    error = can_start();

    if (error == ESP_OK)
    {
        Logger::Notice("CAN Driver Started");
    }

    else
    {
        while (1)
        {
            Logger::Error("CAN Driver Failed to start");
        }
        return;
    }
}

void CanDriver::sendCanData(const char canData, const uint32_t canLen, const uint16_t canID, const int canDATAint, bool isStringmsg = false)
{
    // NOTE: message size caps out around 10, might need to condense data
    can_message_t message;

    message.identifier = canID;
    const charcanMSG = canData;

    message.flags = CAN_MSG_FLAG_NONE; // MSG_FLAG_NONE for actual use
    message.data_length_code = canLen;

    // case for string data
    if (isStringmsg)
    {
        memcpy(message.data, canMSG, message.data_length_code);
    }
    else
    {
        // srand(time(NULL));
        //  for non string data:
        for (int i = 0; i < canLen; i++)
        {
            message.data[i] = (uint8_t)((canDATAint >> (i * 8)) & 0xFF);
            // printf("%d\n", randnum);
            // message.data[i] = canDATAint;
        }
    }

    auto ret = can_transmit(&message, pdMS_TO_TICKS(1000));
    // Queue message for transmission
    if (ret == ESP_OK)
    {
        Logger::Notice("Message queued for transmission");
    }
    else
    {
        Logger::Error("Failed to queue message for transmission. error: 0x%x", ret);
    }

    //
    uint32_t alerts_triggered;

    // TODO: find a way to display/utulize this function
    // At the moment seemingly doesn't work
    can_read_alerts(&alerts_triggered, pdMS_TO_TICKS(1000));
}


// TODO: identify the type of data to recieve
// return data or print?
void CanDriver::readCanData()
{
    can_message_t rx_frame;

    // expecting to read chars - maybe do hex -> char??
    if (can_receive(&rx_frame, pdMS_TO_TICKS(1000)) == ESP_OK)
    {
        Logger::Trace("Data recieved from id:0x%x, len:%d, Data: ", rx_frame.identifier, rx_frame.data_length_code);
        for (int i = 0; i < rx_frame.data_length_code; i++)
        {
            printf("%d", rx_frame.data[i]);
        }
        printf("\n");
    }
    else
    {
        Logger::Error("failed to recieve message");
    }
}