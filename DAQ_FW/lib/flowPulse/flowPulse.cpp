#include "flowPulse.h"
#include "logger.h"
#include "systemConfig.h"
#include <math.h>

namespace {
    volatile uint32_t gFlow1PulseCount = 0;
    volatile uint32_t gFlow2PulseCount = 0;
    portMUX_TYPE gFlowMux = portMUX_INITIALIZER_UNLOCKED;

    float gFlow1Lpm = NAN;
    float gFlow2Lpm = NAN;
    uint32_t gLastSampleMs = 0;
    bool gInitialized = false;

    void IRAM_ATTR flow1Isr() {
        portENTER_CRITICAL_ISR(&gFlowMux);
        ++gFlow1PulseCount;
        portEXIT_CRITICAL_ISR(&gFlowMux);
    }

    void IRAM_ATTR flow2Isr() {
        portENTER_CRITICAL_ISR(&gFlowMux);
        ++gFlow2PulseCount;
        portEXIT_CRITICAL_ISR(&gFlowMux);
    }

    float pulsesToLpm(uint32_t pulses, float samplePeriodSec) {
        if (samplePeriodSec <= 0.0f) {
            return NAN;
        }

        float frequencyHz = static_cast<float>(pulses) / samplePeriodSec;
        return frequencyHz / FLOW_SENSOR_HZ_PER_LPM;
    }
}

void flowPulseInit() {
    portENTER_CRITICAL(&gFlowMux);
    gFlow1PulseCount = 0;
    gFlow2PulseCount = 0;
    gFlow1Lpm = NAN;
    gFlow2Lpm = NAN;
    gLastSampleMs = millis();
    portEXIT_CRITICAL(&gFlowMux);

#if ENABLE_FLOW_SENSOR_1
    pinMode(static_cast<uint8_t>(FLOW_SENSOR_1_PIN), INPUT_PULLUP);
    attachInterrupt(static_cast<uint8_t>(FLOW_SENSOR_1_PIN), flow1Isr, FALLING);
#endif
#if ENABLE_FLOW_SENSOR_2
    pinMode(static_cast<uint8_t>(FLOW_SENSOR_2_PIN), INPUT_PULLUP);
    attachInterrupt(static_cast<uint8_t>(FLOW_SENSOR_2_PIN), flow2Isr, FALLING);
#endif

    gInitialized = true;
    Logger::notice("Flow pulse module initialized (window=%lu ms)",
                   static_cast<unsigned long>(FLOW_SENSOR_SAMPLE_WINDOW_MS));
}

void flowPulseUpdate() {
    if (!gInitialized) {
        return;
    }

    uint32_t nowMs = millis();
    uint32_t elapsedMs = nowMs - gLastSampleMs;
    if (elapsedMs < FLOW_SENSOR_SAMPLE_WINDOW_MS) {
        return;
    }

    uint32_t pulseCount1 = 0;
    uint32_t pulseCount2 = 0;

    portENTER_CRITICAL(&gFlowMux);
    pulseCount1 = gFlow1PulseCount;
    pulseCount2 = gFlow2PulseCount;
    gFlow1PulseCount = 0;
    gFlow2PulseCount = 0;
    gLastSampleMs = nowMs;
    portEXIT_CRITICAL(&gFlowMux);

    float samplePeriodSec = static_cast<float>(elapsedMs) / 1000.0f;

#if ENABLE_FLOW_SENSOR_1
    gFlow1Lpm = pulsesToLpm(pulseCount1, samplePeriodSec);
#else
    gFlow1Lpm = NAN;
#endif

#if ENABLE_FLOW_SENSOR_2
    gFlow2Lpm = pulsesToLpm(pulseCount2, samplePeriodSec);
#else
    gFlow2Lpm = NAN;
#endif
}

float flowPulseGetFlow1Lpm() {
    return gFlow1Lpm;
}

float flowPulseGetFlow2Lpm() {
    return gFlow2Lpm;
}
