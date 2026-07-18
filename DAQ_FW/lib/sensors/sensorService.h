#pragma once

#include "snapshotService.h"

void sensorServiceInit();
void sensorServiceReadCritical(SensorSnapshot &snapshot);
void sensorServiceReadChassis(SensorSnapshot &snapshot);
