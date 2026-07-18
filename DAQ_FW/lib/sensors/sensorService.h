#pragma once

#include "SnapshotService.h"

void SensorService_Init();
void SensorService_ReadCritical(SensorSnapshot &snapshot);
void SensorService_ReadChassis(SensorSnapshot &snapshot);
