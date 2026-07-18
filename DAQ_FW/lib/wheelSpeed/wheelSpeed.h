#ifndef WHEELSPEED_H
#define WHEELSPEED_H

#include <Arduino.h>

void wheelSpeedReset();
void wheelSpeedSetup();
float convertPulsesToSpeed(int pulseCount, float samplePeriodSec);
float getFinalWheelSpeed();
float getWheelSpeedFl();
float getWheelSpeedFr();
float getWheelSpeedRl();
float getWheelSpeedRr();

#endif
