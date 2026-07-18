#ifndef LOGGER_LIB
#define LOGGER_LIB

#include "system_config.h"
#include "ArduinoLog.h"

namespace Logger
{

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

static void PrintLevel(Print *_logOutput, int logLevel);

static void PrintTimestamp(Print *_logOutput);

static void PrintPrefix(Print *_logOutput, int logLevel);

static void PrintSuffix(Print *_logOutput, int logLevel);

#endif