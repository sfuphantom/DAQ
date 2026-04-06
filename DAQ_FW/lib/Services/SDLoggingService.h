#pragma once

#include "SnapshotService.h"

void SDLoggingService_Init(bool sdAvailable);
void SDLoggingService_Append(const SensorSnapshot &snapshot);
