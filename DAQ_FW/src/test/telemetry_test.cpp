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

    while (TELEMETRY_UART.available() > 0)
    {
        String line = TELEMETRY_UART.readStringUntil('\n');
        Serial.print("received telemetry line: ");
        Serial.println(line);
    }

    delay(1000);
}
