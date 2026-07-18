#pragma once

#include <stddef.h>

bool rtcServiceInit();
bool rtcServiceGetBootTimestamp(char *buffer, size_t length);
bool rtcServiceGetCurrentTimestamp(char *buffer, size_t length);
