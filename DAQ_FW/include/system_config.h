#ifndef SYSTEM_CONFIG_H
#define SYSTEM_CONFIG_H

#include <Arduino.h>
#include <stdarg.h>

// I2C Addressess for the ADS1115 connection
enum class ADCAddress
{
    U1 = 0x48,
    U2 = 0x49,
    U3 = 0x4A,
    U4 = 0x4B
};

// DAQ global Log Level Setup Filter
#define CURRENT_LOG_LEVEL LOG_LEVEL_VERBOSE
// Options:
// 0 - LOG_LEVEL_SILENT     no output
// 1 - LOG_LEVEL_FATAL      fatal errors
// 2 - LOG_LEVEL_ERROR      all errors
// 3 - LOG_LEVEL_WARNING    errors, and warnings
// 4 - LOG_LEVEL_NOTICE     errors, warnings and notices
// 5 - LOG_LEVEL_TRACE      errors, warnings, notices & traces
// 6 - LOG_LEVEL_VERBOSE    all

// Defines global decimal places when displaying floats and doubles ('%D' AND '%F')
// param goes to ArduinoLog.cpp line 192: _logOutput->print(va_arg(*args, double), LOG_DECIMAL_PLACES);
#define LOG_DECIMAL_PLACES 5

// Serial baud rate used everywhere in the firmware/monitor
uint32_t constexpr BAUD_RATE = 115200;

// contstant time calculation variables
uint32_t constexpr MSECS_PER_SEC = 1000;
uint32_t constexpr SECS_PER_MIN = 60;
uint32_t constexpr SECS_PER_HOUR = 3600;
uint32_t constexpr SECS_PER_DAY = 86400;

#endif
