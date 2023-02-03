#include "Log.h"

namespace Logger
{

    static void PrintLevel(Print *_logOutput, int logLevel)
    {
        /// Show log description based on log level
        switch (logLevel)
        {
        case 0:
            _logOutput->print("[SILENT] :");
            break;
        case 1:
            _logOutput->print("[FATAL] :");
            break;
        case 2:
            _logOutput->print("[ERROR] :");
            break;
        case 3:
            _logOutput->print("[WARNING] :");
            break;
        case 4:
            _logOutput->print("[INFO] :");
            break;
        case 5:
            _logOutput->print("[TRACE] :");
            break;
        case 6:
            _logOutput->print("[VERBOSE] :");
            break;

        default:
            break;
        }
    }

    static void PrintTimestamp(Print *_logOutput) // unedited ArduinoLog timesamp
    {
        // Total time
        const uint32_t msecs = millis();
        const uint32_t secs = msecs / MSECS_PER_SEC;

        // Time in components
        const uint32_t MilliSeconds = msecs % MSECS_PER_SEC;
        const uint32_t Seconds = secs % SECS_PER_MIN;
        const uint32_t Minutes = (secs / SECS_PER_MIN) % SECS_PER_MIN;
        const uint32_t Hours = (secs % SECS_PER_DAY) / SECS_PER_HOUR;

        // Time as string
        char timestamp[20];
        sprintf(timestamp, "%02d:%02d:%02d.%03d ", Hours, Minutes, Seconds, MilliSeconds);
        _logOutput->print(timestamp);
    }

    static void PrintPrefix(Print *_logOutput, int logLevel)
    {
        PrintTimestamp(_logOutput);
        PrintLevel(_logOutput, logLevel);
    }

    static void PrintSuffix(Print *_logOutput, int logLevel)
    {
        _logOutput->print("");
    }

    // Wrapper function to be called at setup
    void Start()
    {
        Log.setPrefix(PrintPrefix);
        Log.setSuffix(PrintSuffix);
        Log.begin(LOG_LEVEL_VERBOSE, &Serial);
        Log.setShowLevel(false);
        Log.info("ArduinoLog System Start Success" CR);
    }

    void Verbose(const char *msg)
    {
        Log.verbose(msg, CR);
    }

    void Trace(const char *msg)
    {
        Log.trace(msg, CR);
    }

    void Info(const char *msg)
    {
        Log.info(msg, CR);
    }

    void Warning(const char *msg)
    {
        Log.warning(msg, CR);
    }

    void Error(const char *msg)
    {
        Log.error(msg, CR);
    }

    void Fatal(const char *msg)
    {
        Log.fatal(msg, CR);
    }
}
// #define USE_ESP32 uncomment to remove ArduinoLog usecase

// void Logger::log(LogLevel level, const std::string &message);

// Desired Log Template:
// TIME [LOG_LEVEL] : LOG INFO STRING
// 17:20:10:38 [Info] : ArduinoLog System Start Success
// ESP32Log Template current:
// I () DAQ: ESP32log System Start Success
