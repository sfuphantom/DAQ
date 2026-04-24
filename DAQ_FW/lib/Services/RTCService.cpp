#include "RTCService.h"
#include "Logger.h"
#include <RTClib.h>
#include <stdio.h>

namespace
{
    RTC_DS3231 rtc;
    bool rtcReady = false;
    char bootTimestamp[16] = "";
}

bool RTCService_Init()
{
    rtcReady = false;
    bootTimestamp[0] = '\0';

    if (!rtc.begin())
    {
        Logger::Error("RTC init failed");
        return false;
    }

    if (rtc.lostPower())
    {
        Logger::Error("RTC lost power; timestamped log filename disabled");
        return false;
    }

    DateTime now = rtc.now();
    snprintf(
        bootTimestamp,
        sizeof(bootTimestamp),
        "%04u%02u%02u_%02u%02u%02u",
        static_cast<unsigned>(now.year()),
        static_cast<unsigned>(now.month()),
        static_cast<unsigned>(now.day()),
        static_cast<unsigned>(now.hour()),
        static_cast<unsigned>(now.minute()),
        static_cast<unsigned>(now.second()));

    rtcReady = true;
    Logger::Notice("RTC init OK");
    return true;
}

bool RTCService_GetBootTimestamp(char *buffer, size_t length)
{
    if (!rtcReady || buffer == nullptr || length == 0)
    {
        return false;
    }

    snprintf(buffer, length, "%s", bootTimestamp);
    return true;
}
