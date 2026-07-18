#pragma once

#include "snapshotService.h"

void telemetryServiceSendWheelSpeed(float wheelSpeedKmh);
void telemetryServiceSendKnownWheelSpeedTestPattern();
void telemetryServiceSendSnapshotCsv(const SensorSnapshot &snapshot);
