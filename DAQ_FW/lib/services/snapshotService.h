#pragma once

#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/portmacro.h>

struct SensorSnapshot {
    float temp1;
    float temp2;
    float flow1Lpm;
    float flow2Lpm;
    float susp1;
    float susp2;
    float susp3;
    float susp4;
    float steeringAngleDeg;
    float speedFL;
    float speedFR;
    float speedRL;
    float speedRR;
    float speedKmh;
    bool faultActive;
    uint32_t criticalTimestampMs;
    uint32_t chassisTimestampMs;
    uint32_t wheelSpeedTimestampMs;
};

SensorSnapshot snapshotServiceRead();
void snapshotServiceUpdateCritical(float temp1, float temp2, float flow1Lpm, float flow2Lpm, bool faultActive, uint32_t timestampMs);
void snapshotServiceUpdateWheelSpeed(float speedFL, float speedFR, float speedRL, float speedRR, float speedKmh, uint32_t timestampMs);
void snapshotServiceUpdateChassis(float susp1, float susp2, float susp3, float susp4, float steeringAngleDeg, uint32_t timestampMs);
