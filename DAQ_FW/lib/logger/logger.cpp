#include "logger.h"

// Output Log Template:
// TIME [LOG_LEVEL] : LOG INFO STRING
// 17:20:10:38 [Info] : ArduinoLog System Start Success

// uses printf-like structure, ie: Error(format string, data_variables)

// Wrapper function to be called at setup
// Sets up the log message template
void Logger::start() {
    Serial.begin(BAUD_RATE);
    Log.setPrefix(printPrefix);
    Log.setSuffix(printSuffix);
    Log.begin(CURRENT_LOG_LEVEL, &Serial, false);
    Log.infoln("ArduinoLog System Start Success");
}

static void printLevel(Print *_logOutput, int logLevel) {
    /// Show log description based on log level
    switch (logLevel) {
    case 0:
        _logOutput->print("[SILENT] : ");
        break;
    case 1:
        _logOutput->print("[FATAL] : ");
        break;
    case 2:
        _logOutput->print("[ERROR] : ");
        break;
    case 3:
        _logOutput->print("[WARNING] : ");
        break;
    case 4:
        _logOutput->print("[Notice] : ");
        break;
    case 5:
        _logOutput->print("[TRACE] : ");
        break;
    case 6:
        _logOutput->print("[VERBOSE] : ");
        break;

    default:
        break;
    }
}

static void printTimestamp(Print *_logOutput) {
    // unedited ArduinoLog timesamp
    // Total time
    const uint32_t msecs = millis();
    const uint32_t secs = msecs / 1000U;

    // Time in components
    const uint32_t MilliSeconds = msecs % 1000U;
    const uint32_t Seconds = secs % 60U;
    const uint32_t Minutes = (secs / 60U) % 60U;
    const uint32_t Hours = (secs % 86400U) / 3600U;

    // Time as string
    char timestamp[20];
    sprintf(timestamp, "%02d:%02d:%02d.%03d ", Hours, Minutes, Seconds, MilliSeconds);
    _logOutput->print(timestamp);
}

static void printPrefix(Print *_logOutput, int logLevel) {
    printTimestamp(_logOutput);
    printLevel(_logOutput, logLevel);
}

static void printSuffix(Print *_logOutput, int logLevel) {
    _logOutput->print(" ");
}
