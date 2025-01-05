#include <SD.h>
#include <SPI.h>
#include <Arduino.h>

#define SD_CS_PIN 5 

void setup() {
  // Initialize Serial
  Serial.begin(9600);
  while (!Serial) { // Wait for Serial to be ready
    delay(100);
  }
}

