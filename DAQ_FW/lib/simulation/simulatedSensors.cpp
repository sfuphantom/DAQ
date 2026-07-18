#include "simulatedSensors.h"
#include "systemConfig.h"
#include <math.h>

// This file populates a SensorSnapshot and periodically injects fault conditions to test system behavior.
namespace {
    float triangleWave(uint32_t nowMs, uint32_t periodMs, float minValue, float maxValue){
        if (periodMs == 0){
            return minValue;
        }

        float phase = static_cast<float>(nowMs % periodMs) / static_cast<float>(periodMs);
        float ramp = phase < 0.5f ? (phase * 2.0f) : (2.0f - phase * 2.0f);
        return minValue + (maxValue - minValue) * ramp;
    }

    float sinWave(uint32_t nowMs, uint32_t periodMs, float center, float amplitude){
        if (periodMs == 0){
            return center;
        }

        float phase = (static_cast<float>(nowMs % periodMs) / static_cast<float>(periodMs)) * 2.0f * PI;
        return center + amplitude * sinf(phase);
    }
}

void simulatedSensorsFillSnapshot(SensorSnapshot &snapshot, uint32_t nowMs){
    uint32_t cycleMs = nowMs % 20000UL;
    bool induceFault = cycleMs >= 12000UL && cycleMs < 16000UL;

    snapshot.temp1 = induceFault ? (MAX_TEMP_1 + 6.0f) : sinWave(nowMs, 9000, 42.0f, 6.0f);
    snapshot.temp2 = sinWave(nowMs + 1700UL, 11000, 39.0f, 5.0f);
    snapshot.flow1Lpm = induceFault ? 0.0f : triangleWave(nowMs, 7000, 8.0f, 18.0f);
    snapshot.flow2Lpm = triangleWave(nowMs + 1300UL, 8000, 7.5f, 16.5f);

    snapshot.susp1 = triangleWave(nowMs, 3000, 0.7f, 3.2f);
    snapshot.susp2 = triangleWave(nowMs + 500UL, 3400, 0.9f, 3.1f);
    snapshot.susp3 = triangleWave(nowMs + 1000UL, 3800, 0.8f, 3.0f);
    snapshot.susp4 = triangleWave(nowMs + 1500UL, 4200, 0.6f, 3.3f);

    snapshot.steeringAngleDeg = triangleWave(nowMs, 6000, 0.0f, STEERING_ANGLE_FULL_SCALE_DEG);

    snapshot.speedKmh = triangleWave(nowMs, 10000, 0.0f, 80.0f);
    snapshot.speedFL = snapshot.speedKmh + 0.4f;
    snapshot.speedFR = snapshot.speedKmh - 0.3f;
    snapshot.speedRL = snapshot.speedKmh + 0.2f;
    snapshot.speedRR = snapshot.speedKmh - 0.1f;
}
