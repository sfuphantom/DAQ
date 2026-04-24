#pragma once

#include "SnapshotService.h"

void SDLoggingService_Init(bool sdAvailable, const char *runTimestamp = nullptr);
void SDLoggingService_Append(const SensorSnapshot &snapshot);
