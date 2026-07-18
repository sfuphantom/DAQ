#include <Arduino.h>
#include "systemConfig.h"

void setup() {
    Serial.begin(BAUD_RATE);
    delay(1000);

    Serial.println("Idle power test");
    Serial.println("No I2C, SPI, CAN, telemetry UART, SD, or sensor tasks are initialized.");
}

void loop() {
    delay(1000);
}
