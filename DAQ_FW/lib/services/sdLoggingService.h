#pragma once

#include "snapshotService.h"

void sdLoggingServiceInit(bool sdAvailable, const char *runTimestamp = nullptr);
void sdLoggingServiceAppend(const SensorSnapshot &snapshot);
const char *sdLoggingServiceLogPath();
