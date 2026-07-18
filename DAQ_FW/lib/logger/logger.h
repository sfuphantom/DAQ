#ifndef LOGGER_LIB
#define LOGGER_LIB

#include "systemConfig.h"
#include "ArduinoLog.h"

namespace Logger {

    void start();

    template <class T, typename... Args>
    void verbose(T msg, Args... args) {
        Log.verboseln(msg, args...);
    }

    template <class T, typename... Args>
    void trace(T msg, Args... args) {
        Log.traceln(msg, args...);
    }

    template <class T, typename... Args>
    void notice(T msg, Args... args) {
        Log.noticeln(msg, args...);
    }

    template <class T, typename... Args>
    void warning(T msg, Args... args) {
        Log.warningln(msg, args...);
    }

    template <class T, typename... Args>
    void error(T msg, Args... args) {
        Log.errorln(msg, args...);
    }

    template <class T, typename... Args>
    void fatal(T msg, Args... args) {
        Log.fatalln(msg, args...);
    }

}

static void printLevel(Print *_logOutput, int logLevel);

static void printTimestamp(Print *_logOutput);

static void printPrefix(Print *_logOutput, int logLevel);

static void printSuffix(Print *_logOutput, int logLevel);

#endif