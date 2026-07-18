#include <Arduino.h>
#include <Wire.h>
#include <RTClib.h>
#include "systemConfig.h"

RTC_DS3231 rtc;

void setup() {
    Serial.begin(BAUD_RATE);
    delay(1000);

    Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);
    Serial.println("RTC set-time test started");
    Serial.printf("SDA=%d SCL=%d\n", I2C_SDA_PIN, I2C_SCL_PIN);

    if (!rtc.begin()) {
        Serial.println("RTC init failed");
        return;
    }

    DateTime compileTime(F(__DATE__), F(__TIME__));
    rtc.adjust(compileTime);
    Serial.println("RTC adjusted to firmware compile time");
    Serial.printf(
        "Compile time: %04d-%02d-%02d %02d:%02d:%02d\n",
        compileTime.year(),
        compileTime.month(),
        compileTime.day(),
        compileTime.hour(),
        compileTime.minute(),
        compileTime.second());

    if (rtc.lostPower()) {
        Serial.println("RTC still reports lost power after adjust");
    }
    else {
        Serial.println("RTC power OK after adjust");
    }
}

void loop() {
    DateTime now = rtc.now();
    Serial.printf(
        "ESP millis=%lu RTC now: %04d-%02d-%02d %02d:%02d:%02d\n",
        static_cast<unsigned long>(millis()),
        now.year(),
        now.month(),
        now.day(),
        now.hour(),
        now.minute(),
        now.second());
    delay(1000);
}
