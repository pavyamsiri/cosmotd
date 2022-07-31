#pragma once

// Standard libraries
#include <iostream>
#include <sstream>
#include <time.h>

// External libraries

// Internal libraries

enum class LogLevel
{
    TRACE = 0,
    DEBUG,
    INFO,
    WARNING,
    ERROR,
    FATAL,
};

void logTrace(const char *message);
void logDebug(const char *message);
void logInfo(const char *message);
void logWarning(const char *message);
void logError(const char *message);
void logFatal(const char *message);
