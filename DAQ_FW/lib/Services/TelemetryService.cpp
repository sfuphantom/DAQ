#include "TelemetryService.h"
#include "Logger.h"
#include "can.h"
#include "system_config.h"
#include <Arduino.h>
#include <math.h>
#include <stdio.h>

static const char *formatValue(float value, char *buffer, size_t length, uint8_t precision)
{
    if (isnan(value))
    {
        snprintf(buffer, length, "null");
    }
    else
    {
        snprintf(buffer, length, "%.*f", precision, value);
    }
    return buffer;
}

void TelemetryService_SendWheelSpeed(float wheelSpeedKmh)
{
    int16_t speedCentiKmh = static_cast<int16_t>(wheelSpeedKmh * 100.0f);
    CAN_SendInt16(static_cast<uint16_t>(CANMessageId::WheelSpeed), speedCentiKmh);
    Serial.println();
    Logger::Trace("[WheelSpeed] %.2f km/h (centi=%d)", wheelSpeedKmh, speedCentiKmh);
    Serial.println();
}

void TelemetryService_SendKnownWheelSpeedTestPattern()
{
    static const float speeds[] = {0.00f, 10.0f, 30.0f, 60.0f, 100.0f};
    static int idx = 0;
    static unsigned long lastSend = 0;

    unsigned long now = millis();
    if (now - lastSend >= 500) {
        float s = speeds[idx];
        TelemetryService_SendWheelSpeed(s);
        idx = (idx + 1) % (sizeof(speeds) / sizeof(speeds[0]));
        lastSend = now;
    }
}

void TelemetryService_SendSnapshotCSV(const SensorSnapshot &snapshot)
{
    char temp1Buffer[16];
    char temp2Buffer[16];
    char flow1Buffer[16];
    char flow2Buffer[16];
    char susp1Buffer[16];
    char susp2Buffer[16];
    char susp3Buffer[16];
    char susp4Buffer[16];
    char steeringAngleBuffer[16];
    char speedBuffer[16];

    TELEMETRY_UART.print(snapshot.timestampMs);
    TELEMETRY_UART.print(",");
    TELEMETRY_UART.print(formatValue(snapshot.temp1, temp1Buffer, sizeof(temp1Buffer), 1));
    TELEMETRY_UART.print(",");
    TELEMETRY_UART.print(formatValue(snapshot.temp2, temp2Buffer, sizeof(temp2Buffer), 1));
    TELEMETRY_UART.print(",");
    TELEMETRY_UART.print(formatValue(snapshot.flow1Lpm, flow1Buffer, sizeof(flow1Buffer), 2));
    TELEMETRY_UART.print(",");
    TELEMETRY_UART.print(formatValue(snapshot.flow2Lpm, flow2Buffer, sizeof(flow2Buffer), 2));
    TELEMETRY_UART.print(",");
    TELEMETRY_UART.print(formatValue(snapshot.susp1, susp1Buffer, sizeof(susp1Buffer), 3));
    TELEMETRY_UART.print(",");
    TELEMETRY_UART.print(formatValue(snapshot.susp2, susp2Buffer, sizeof(susp2Buffer), 3));
    TELEMETRY_UART.print(",");
    TELEMETRY_UART.print(formatValue(snapshot.susp3, susp3Buffer, sizeof(susp3Buffer), 3));
    TELEMETRY_UART.print(",");
    TELEMETRY_UART.print(formatValue(snapshot.susp4, susp4Buffer, sizeof(susp4Buffer), 3));
    TELEMETRY_UART.print(",");
    TELEMETRY_UART.print(formatValue(snapshot.steeringAngleDeg, steeringAngleBuffer, sizeof(steeringAngleBuffer), 1));
    TELEMETRY_UART.print(",");
    TELEMETRY_UART.println(formatValue(snapshot.speedKmh, speedBuffer, sizeof(speedBuffer), 2));
}
