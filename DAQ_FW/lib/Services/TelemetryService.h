#pragma once

#include "SnapshotService.h"

void TelemetryService_SendWheelSpeed(float wheelSpeedKmh);
void TelemetryService_SendKnownWheelSpeedTestPattern();
void TelemetryService_SendSnapshotCSV(const SensorSnapshot &snapshot);
