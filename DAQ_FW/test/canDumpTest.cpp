#include <Arduino.h>
#include <driver/twai.h>
#include <esp_err.h>
#include "systemConfig.h"

static bool canReady = false;

void setup() {
    Serial.begin(BAUD_RATE);
    delay(1000);

    Serial.println("CAN dump test");
    Serial.printf("TX=%d RX=%d bitrate=500k\n", CAN_TX_PIN, CAN_RX_PIN);

    twai_general_config_t generalConfig = TWAI_GENERAL_CONFIG_DEFAULT(CAN_TX_PIN, CAN_RX_PIN, TWAI_MODE_NORMAL);
    generalConfig.tx_queue_len = 0;
    generalConfig.rx_queue_len = 32;

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
    Serial.println("CAN ready; dumping frames");
}

void loop() {
    if (!canReady) {
        delay(1000);
        return;
    }

    twai_message_t message = {};
    esp_err_t rxResult = twai_receive(&message, pdMS_TO_TICKS(1000));
    if (rxResult != ESP_OK) {
        return;
    }

    Serial.printf(
        "ID=0x%03X DLC=%d EXT=%d RTR=%d DATA=",
        message.identifier,
        message.data_length_code,
        message.extd ? 1 : 0,
        message.rtr ? 1 : 0);

    for (int i = 0; i < message.data_length_code; ++i) {
        Serial.printf("%02X", message.data[i]);
        if (i + 1 < message.data_length_code) {
            Serial.print(" ");
        }
    }

    Serial.println();
}
