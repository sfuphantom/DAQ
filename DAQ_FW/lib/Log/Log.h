#ifndef LOG_LIB
#define LOG_LIB

#include <Arduino.h>
#include "ArduinoLog.h"

#define LOG_LOCAL_LEVEL ESP_LOG_VERBOSE
#include "esp_log.h"

namespace Logger
{

    uint32_t constexpr MSECS_PER_SEC = 1000;
    uint32_t constexpr SECS_PER_MIN = 60;
    uint32_t constexpr SECS_PER_HOUR = 3600;
    uint32_t constexpr SECS_PER_DAY = 86400;

    void Start();

    void Verbose(const char *msg);

    void Trace(const char *msg);

    void Info(const char *msg);

    void Warning(const char *msg);

    void Error(const char *msg);

    void Fatal(const char *msg);
}
#endif