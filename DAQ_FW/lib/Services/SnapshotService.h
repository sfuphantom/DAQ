#pragma once

#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/portmacro.h>

struct SensorSnapshot
{
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
    uint32_t timestampMs;
};

SensorSnapshot SnapshotService_Read();
void SnapshotService_UpdateCritical(float temp1, float temp2, float flow1Lpm, float flow2Lpm, bool faultActive, uint32_t timestampMs);
void SnapshotService_UpdateWheelSpeed(float speedFL, float speedFR, float speedRL, float speedRR, float speedKmh, uint32_t timestampMs);
void SnapshotService_UpdateChassis(float susp1, float susp2, float susp3, float susp4, float steeringAngleDeg, uint32_t timestampMs);
