#include <Arduino.h>
#include <Wire.h>
#include "systemConfig.h"

static const char *knownDeviceName(uint8_t address) {
    switch (address) {
    case 0x48:
    case 0x49:
    case 0x4A:
    case 0x4B:
        return "ADS1115 candidate";
    case 0x68:
        return "DS3231 RTC candidate";
    default:
        return "";
    }
}

void setup() {
    Serial.begin(BAUD_RATE);
    delay(1000);

    Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);
    Serial.println("I2C scan started");
    Serial.printf("SDA=%d SCL=%d\n", I2C_SDA_PIN, I2C_SCL_PIN);
}

void loop() {
    uint8_t foundCount = 0;

    for (uint8_t address = 1; address < 127; ++address) {
        Wire.beginTransmission(address);
        uint8_t error = Wire.endTransmission();

        if (error == 0) {
            const char *name = knownDeviceName(address);
            Serial.printf("I2C device found at 0x%02X", address);
            if (name[0] != '\0') {
                Serial.printf(" (%s)", name);
            }
            Serial.println();
            ++foundCount;
        }
    }

    if (foundCount == 0) {
        Serial.println("No I2C devices found");
    }
    else {
        Serial.printf("I2C scan complete: %u device(s)\n", foundCount);
    }

    delay(3000);
}
