#include <Arduino.h>
#include "systemConfig.h"

static constexpr uint8_t kTestPin = 4;

void setup() {
    Serial.begin(BAUD_RATE);
    delay(1000);

    pinMode(kTestPin, OUTPUT);
    digitalWrite(kTestPin, HIGH);

    Serial.println("GPIO4 high test");
    Serial.println("GPIO4 is held HIGH continuously.");
}

void loop() {
    digitalWrite(kTestPin, HIGH);
    delay(1000);
    digitalWrite(kTestPin, LOW);
    delay(1000);
}
