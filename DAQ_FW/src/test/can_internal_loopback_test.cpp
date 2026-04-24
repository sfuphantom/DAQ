#include <Arduino.h>
#include <driver/twai.h>
#include <esp_err.h>
#include "system_config.h"

static bool canReady = false;

void setup()
{
    Serial.begin(BAUD_RATE);
    delay(1000);

    Serial.println("CAN internal loopback test");
    Serial.println("This validates the ESP32 TWAI peripheral without the external transceiver.");
    Serial.printf("TX=%d RX=%d bitrate=500k mode=no-ack self-reception\n", CAN_TX_PIN, CAN_RX_PIN);

    twai_general_config_t generalConfig = TWAI_GENERAL_CONFIG_DEFAULT(CAN_TX_PIN, CAN_RX_PIN, TWAI_MODE_NO_ACK);
    generalConfig.tx_queue_len = 10;
    generalConfig.rx_queue_len = 10;
    generalConfig.alerts_enabled = TWAI_ALERT_ALL;

    twai_timing_config_t timingConfig = TWAI_TIMING_CONFIG_500KBITS();
    twai_filter_config_t filterConfig = TWAI_FILTER_CONFIG_ACCEPT_ALL();

    esp_err_t installResult = twai_driver_install(&generalConfig, &timingConfig, &filterConfig);
    if (installResult != ESP_OK)
    {
        Serial.printf("CAN driver install failed: %s\n", esp_err_to_name(installResult));
        return;
    }

    esp_err_t startResult = twai_start();
    if (startResult != ESP_OK)
    {
        Serial.printf("CAN driver start failed: %s\n", esp_err_to_name(startResult));
        return;
    }

    canReady = true;
    Serial.println("CAN driver started");
}

void loop()
{
    if (!canReady)
    {
        Serial.println("CAN driver not ready");
        delay(1000);
        return;
    }

    twai_message_t message = {};
    message.identifier = 0x123;
    message.data_length_code = 2;
    message.self = 1;
    message.data[0] = 0xCA;
    message.data[1] = 0xFE;

    esp_err_t txResult = twai_transmit(&message, pdMS_TO_TICKS(100));
    if (txResult != ESP_OK)
    {
        Serial.printf("Internal loopback TX failed: %s\n", esp_err_to_name(txResult));
        delay(1000);
        return;
    }

    twai_message_t received = {};
    esp_err_t rxResult = twai_receive(&received, pdMS_TO_TICKS(1000));
    if (rxResult == ESP_OK)
    {
        Serial.printf(
            "Internal loopback OK id=0x%03X data=%02X %02X\n",
            received.identifier,
            received.data[0],
            received.data[1]);
    }
    else
    {
        Serial.printf("Internal loopback RX failed: %s\n", esp_err_to_name(rxResult));
    }

    delay(1000);
}
