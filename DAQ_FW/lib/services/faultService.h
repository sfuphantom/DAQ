#pragma once

#include <Arduino.h>

void FaultService_Init(uint32_t startupTimeMs);
bool FaultService_Update(float flow1Lpm, float flow2Lpm, float temp1, float temp2, bool canEnabled);
