#include <Arduino.h>
#include <SPI.h>
#include <SD.h>
#include "systemConfig.h"

// NOTE: This only proves SD wiring/card/file write works.

static const char *kPath = "/sd_test.txt";

void setup() {
    Serial.begin(BAUD_RATE);
    delay(1000);

    SPI.begin(SD_SCK_PIN, SD_MISO_PIN, SD_MOSI_PIN, SD_CS_PIN);
    if (!SD.begin(SD_CS_PIN, SPI)) {
        Serial.println("SD init failed");
        return;
    }
    Serial.println("SD init OK");

    File file = SD.open(kPath, FILE_WRITE);
    if (!file) {
        Serial.println("SD open for write failed");
        return;
    }
    file.println("sd test write ok");
    file.close();
    Serial.println("SD write OK");

    file = SD.open(kPath, FILE_READ);
    if (!file) {
        Serial.println("SD open for read failed");
        return;
    }
    Serial.println("SD read:");
    while (file.available()) {
        Serial.write(file.read());
    }
    file.close();
}

void loop() {
    delay(1000);
}
