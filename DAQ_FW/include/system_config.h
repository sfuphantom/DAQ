#ifndef SYSTEM_CONFIG_H
#define SYSTEM_CONFIG_H

#include <Arduino.h>
#include <stdarg.h>


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

// Baud Rate variable
uint32_t constexpr BAUD_RATE = 9600;

// contstant time calculation variables
uint32_t constexpr MSECS_PER_SEC = 1000;
uint32_t constexpr SECS_PER_MIN = 60;
uint32_t constexpr SECS_PER_HOUR = 3600;
uint32_t constexpr SECS_PER_DAY = 86400;

#endif