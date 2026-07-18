#include "loggingService.h"
#include "logger.h"
#include "systemConfig.h"
#include <Arduino.h>
#include <math.h>
#include <stdio.h>

static const char *formatSensorValue(float value, char *buffer, size_t length, uint8_t precision) {
    if (isnan(value)) {
        snprintf(buffer, length, "%s", SENSOR_NULL_TEXT);
    }
    else {
        snprintf(buffer, length, "%.*f", precision, value);
    }
    return buffer;
}

void loggingServiceLogSnapshot(const SensorSnapshot &snapshot) {
    char temp1Buffer[16];
    char temp2Buffer[16];
    char flow1Buffer[16];
    char flow2Buffer[16];
    char susp1Buffer[16];
    char susp2Buffer[16];
    char susp3Buffer[16];
    char susp4Buffer[16];
    char steeringAngleBuffer[16];
    char speedFLBuffer[16];
    char speedFRBuffer[16];
    char speedRLBuffer[16];
    char speedRRBuffer[16];
    char speedBuffer[16];

    Serial.println();
    Logger::notice("[Data] Timestamps ms: critical=%lu chassis=%lu wheel=%lu, Temp1: %s C, Temp2: %s C, Flow1: %s L/min, Flow2: %s L/min, Susp V: [%s %s %s %s], Steering: %s deg, Wheels: [%s %s %s %s], Speed: %s km/h, Fault: %d",
                   static_cast<unsigned long>(snapshot.criticalTimestampMs),
                   static_cast<unsigned long>(snapshot.chassisTimestampMs),
                   static_cast<unsigned long>(snapshot.wheelSpeedTimestampMs),
                   formatSensorValue(snapshot.temp1, temp1Buffer, sizeof(temp1Buffer), 1),
                   formatSensorValue(snapshot.temp2, temp2Buffer, sizeof(temp2Buffer), 1),
                   formatSensorValue(snapshot.flow1Lpm, flow1Buffer, sizeof(flow1Buffer), 2),
                   formatSensorValue(snapshot.flow2Lpm, flow2Buffer, sizeof(flow2Buffer), 2),
                   formatSensorValue(snapshot.susp1, susp1Buffer, sizeof(susp1Buffer), 3),
                   formatSensorValue(snapshot.susp2, susp2Buffer, sizeof(susp2Buffer), 3),
                   formatSensorValue(snapshot.susp3, susp3Buffer, sizeof(susp3Buffer), 3),
                   formatSensorValue(snapshot.susp4, susp4Buffer, sizeof(susp4Buffer), 3),
                   formatSensorValue(snapshot.steeringAngleDeg, steeringAngleBuffer, sizeof(steeringAngleBuffer), 1),
                   formatSensorValue(snapshot.speedFL, speedFLBuffer, sizeof(speedFLBuffer), 2),
                   formatSensorValue(snapshot.speedFR, speedFRBuffer, sizeof(speedFRBuffer), 2),
                   formatSensorValue(snapshot.speedRL, speedRLBuffer, sizeof(speedRLBuffer), 2),
                   formatSensorValue(snapshot.speedRR, speedRRBuffer, sizeof(speedRRBuffer), 2),
                   formatSensorValue(snapshot.speedKmh, speedBuffer, sizeof(speedBuffer), 2),
                   snapshot.faultActive ? 1 : 0);
    Serial.println();
}
