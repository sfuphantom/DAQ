#include <Arduino.h>
#include <driver/twai.h>
#include <esp_err.h>
#include "systemConfig.h"

static bool canReady = false;
static uint16_t counter = 0;

void setup() {
    Serial.begin(BAUD_RATE);
    delay(1000);

    Serial.println("CAN simple send test");
    Serial.printf("TX=%d RX=%d bitrate=500k\n", CAN_TX_PIN, CAN_RX_PIN);

    twai_general_config_t generalConfig = TWAI_GENERAL_CONFIG_DEFAULT(CAN_TX_PIN, CAN_RX_PIN, TWAI_MODE_NO_ACK);
    generalConfig.tx_queue_len = 5;
    generalConfig.rx_queue_len = 5;

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
    Serial.println("CAN ready");
}

void loop() {
    if (!canReady) {
        delay(1000);
        return;
    }

    twai_message_t message = {};
    message.identifier = 0x123;
    message.data_length_code = 2;
    message.data[0] = counter & 0xFF;
    message.data[1] = counter >> 8;

    esp_err_t txResult = twai_transmit(&message, pdMS_TO_TICKS(100));
    if (txResult == ESP_OK) {
        Serial.printf("TX ID=0x%03X DLC=2 DATA=%02X %02X\n", message.identifier, message.data[0], message.data[1]);
        ++counter;
    }
    else {
        Serial.printf("TX failed: %s\n", esp_err_to_name(txResult));
    }

    delay(10);
}
