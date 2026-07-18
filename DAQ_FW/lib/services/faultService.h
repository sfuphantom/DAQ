#pragma once

#include <Arduino.h>

void faultServiceInit(uint32_t startupTimeMs);
bool faultServiceUpdate(float flow1Lpm, float flow2Lpm, float temp1, float temp2, bool canEnabled);
