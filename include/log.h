#pragma once

// Standard libraries
#include <iostream>
#include <sstream>
#include <time.h>

// External libraries

// Internal libraries

enum class LogLevel
{
    CLOOP = 0,
    CTRACE = 1,
    CDEBUG = 2,
    CINFO = 3,
    CWARNING = 4,
    CERROR = 5,
    CFATAL = 6,
};

void logWithLevel(LogLevel level, const char *messageFormat, ...);

#ifdef LOG_LEVEL_LOOP
#define logLoop(messageFormat, ...) logWithLevel(LogLevel::CLOOP, messageFormat, ##__VA_ARGS__);
#else
#define logLoop(messageFormat, ...)
#endif

#ifdef LOG_LEVEL_TRACE
#define logTrace(messageFormat, ...) logWithLevel(LogLevel::CTRACE, messageFormat, ##__VA_ARGS__);
#else
#define logTrace(messageFormat, ...)
#endif

#ifdef LOG_LEVEL_DEBUG
#define logDebug(messageFormat, ...) logWithLevel(LogLevel::CDEBUG, messageFormat, ##__VA_ARGS__);
#else
#define logDebug(messageFormat, ...)
#endif

#ifdef LOG_LEVEL_INFO
#define logInfo(messageFormat, ...) logWithLevel(LogLevel::CINFO, messageFormat, ##__VA_ARGS__);
#else
#define logInfo(messageFormat, ...)
#endif

#ifdef LOG_LEVEL_WARNING
#define logWarning(messageFormat, ...) logWithLevel(LogLevel::CWARNING, messageFormat, ##__VA_ARGS__);
#else
#define logWarning(messageFormat, ...)
#endif

#ifdef LOG_LEVEL_ERROR
#define logError(messageFormat, ...) logWithLevel(LogLevel::CERROR, messageFormat, ##__VA_ARGS__);
#else
#define logError(messageFormat, ...)
#endif

#ifdef LOG_LEVEL_FATAL
#define logFatal(messageFormat, ...) logWithLevel(LogLevel::CFATAL, messageFormat, ##__VA_ARGS__);
#else
#define logFatal(messageFormat, ...)
#endif