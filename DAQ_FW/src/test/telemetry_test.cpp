#include <Arduino.h>
#include "system_config.h"

// antenna

void setup()
{
    Serial.begin(BAUD_RATE);
    delay(1000);

    TELEMETRY_UART.begin(TELEMETRY_BAUD, SERIAL_8N1, TELEMETRY_RX_PIN, TELEMETRY_TX_PIN);
    Serial.println("Telemetry UART init OK");
}

void loop()
{
    TELEMETRY_UART.println("telemetry test ping");
    Serial.println("sent telemetry test ping");
    delay(1000);
}
