#include <Arduino.h>
#include <Wire.h>
#include <RTClib.h>
#include "system_config.h"

RTC_DS3231 rtc;

void setup()
{
    Serial.begin(BAUD_RATE);
    delay(1000);

    Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);
    Serial.println("RTC test started");
    Serial.printf("SDA=%d SCL=%d\n", I2C_SDA_PIN, I2C_SCL_PIN);

    if (!rtc.begin())
    {
        Serial.println("RTC init failed");
        return;
    }

    Serial.println("RTC init OK");

    if (rtc.lostPower())
    {
        Serial.println("RTC lost power");
    }
    else
    {
        Serial.println("RTC power OK");
    }
}

void loop()
{
    DateTime now = rtc.now();
    Serial.printf(
        "RTC now: %04d-%02d-%02d %02d:%02d:%02d\n",
        now.year(),
        now.month(),
        now.day(),
        now.hour(),
        now.minute(),
        now.second());
    delay(1000);
}
