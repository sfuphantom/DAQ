#ifndef WHEELSPEED_H
#define WHEELSPEED_H

#include <Arduino.h>

void wheelSpeedDisplay(int volatile WP1, int volatile WP2, int volatile WP3, int volatile WP4);
void WheelSpeedReset();
void WheelSpeedSetup();
float convertPulsesToSpeed(int pulseCount, float samplePeriodSec);
float getFinalWheelSpeed();
float getWheelSpeedFL();
float getWheelSpeedFR();
float getWheelSpeedRL();
float getWheelSpeedRR();

#endif
