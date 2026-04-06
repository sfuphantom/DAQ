#include "FaultService.h"
#include "Logger.h"
#include "can.h"
#include "system_config.h"
#include <math.h>

namespace
{
    bool faultReported = false;
    uint32_t startupTimeMs = 0;
    uint32_t faultStartMs = 0;
    bool faultPending = false;
    const uint32_t kWarmupMs = 15000;
    const uint32_t kFaultDebounceMs = 1000;
}

static bool valueOutOfRange(float value, float minValue, float maxValue)
{
    if (isnan(value)) return false;
    return value < minValue || value > maxValue;
}

void FaultService_Init(uint32_t startupTimeMsValue)
{
    startupTimeMs = startupTimeMsValue;
    faultReported = false;
}

bool FaultService_Update(float flow1Lpm, float flow2Lpm, float temp1, float temp2, bool canEnabled)
{
    bool warmupComplete = (millis() - startupTimeMs) >= kWarmupMs;
    bool rawFaultDetected = valueOutOfRange(flow1Lpm, MIN_FLOW_LPM, MAX_FLOW_LPM) ||
                            valueOutOfRange(flow2Lpm, MIN_FLOW_LPM, MAX_FLOW_LPM) ||
                            valueOutOfRange(temp1, MIN_TEMP, MAX_TEMP_1) ||
                            valueOutOfRange(temp2, MIN_TEMP, MAX_TEMP_2);

    bool faultDetected = false;
    if (warmupComplete && rawFaultDetected)
    {
        if (!faultPending)
        {
            faultPending = true;
            faultStartMs = millis();
        }
        faultDetected = (millis() - faultStartMs) >= kFaultDebounceMs;
    }
    else
    {
        faultPending = false;
        faultStartMs = 0;
    }

    if (faultDetected)
    {
        if (!faultReported)
        {
            Serial.println();
            Logger::Error("FAULT DETECTED: coolant values outside limits");
        }
        if (canEnabled)
        {
            CAN_SendUInt8(static_cast<uint16_t>(CANMessageId::CoolingFault), 1);
        }
    }
    else
    {
        if (faultReported)
        {
            Serial.println();
            Logger::Notice("Coolant values back within range");
        }
        if (canEnabled)
        {
            CAN_SendUInt8(static_cast<uint16_t>(CANMessageId::CoolingFault), 0);
        }
    }
    faultReported = faultDetected;
    return faultDetected;
}
