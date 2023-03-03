#ifndef LOG_LIB
#define LOG_LIB

#include <Arduino.h>
#include <stdarg.h>
#include "ArduinoLog.h"

namespace Logger
{

    uint32_t constexpr MSECS_PER_SEC = 1000;
    uint32_t constexpr SECS_PER_MIN = 60;
    uint32_t constexpr SECS_PER_HOUR = 3600;
    uint32_t constexpr SECS_PER_DAY = 86400;

    void Start();

    template <class T, typename... Args>
    void Verbose(T msg, Args... args)
    {
        Log.verboseln(msg, args...);
    }

    template <class T, typename... Args>
    void Trace(T msg, Args... args)
    {
        Log.traceln(msg, args...);
    }

    template <class T, typename... Args>
    void Notice(T msg, Args... args)
    {
        Log.noticeln(msg, args...);
    }

    template <class T, typename... Args>
    void Warning(T msg, Args... args)
    {
        Log.warningln(msg, args...);
    }

    template <class T, typename... Args>
    void Error(T msg, Args... args)
    {
        Log.errorln(msg, args...);
    }

    template <class T, typename... Args>
    void Fatal(T msg, Args... args)
    {
        Log.fatalln(msg, args...);
    }
}
#endif