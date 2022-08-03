// Standard libraries
#include <iostream>
#include <format>

// Internal libraries
#include "log.h"

constexpr size_t MAX_LOG_SIZE = 32000;

// Windows implemnentation with colored output
#ifdef _WIN32
#include <windows.h>
#include <wincon.h>

static uint8_t logLevelColors[6] = {
    FOREGROUND_BLUE,
    FOREGROUND_GREEN,
    FOREGROUND_BLUE | FOREGROUND_GREEN,
    FOREGROUND_GREEN | FOREGROUND_RED,
    FOREGROUND_RED};
constexpr uint8_t DEFAULT_COLOR = 7;

void logWithLevel(LogLevel level, const char *messageFormat, ...)
{
    static const char *logLevelStrings[6] = {
        "[ TRACE ]: ",
        "[ DEBUG ]: ",
        "[ INFO  ]: ",
        "[WARNING]: ",
        "[ ERROR ]: ",
        "[ FATAL ]: ",
    };

    char buffer[MAX_LOG_SIZE];
    memset(buffer, 0, sizeof(buffer));

    __builtin_va_list argPointer;
    va_start(argPointer, messageFormat);
    vsnprintf_s(buffer, MAX_LOG_SIZE, messageFormat, argPointer);
    va_end(argPointer);

    std::stringstream outStream;
    outStream << logLevelStrings[(size_t)level] << buffer << "\n";

    HANDLE hConsole;
    switch (level)
    {
    case LogLevel::CTRACE:
    case LogLevel::CDEBUG:
    case LogLevel::CINFO:
    case LogLevel::CWARNING:
        hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
        break;
    case LogLevel::CERROR:
    case LogLevel::CFATAL:
        hConsole = GetStdHandle(STD_ERROR_HANDLE);
        break;
    }

    SetConsoleTextAttribute(hConsole, logLevelColors[(size_t)level]);
    WriteConsoleA(hConsole, outStream.str().c_str(), (DWORD)outStream.str().length(), nullptr, NULL);
    SetConsoleTextAttribute(hConsole, DEFAULT_COLOR);
}

#else

void logWithLevel(LogLevel level, const char *messageFormat, ...)
{
    static const char *logLevelStrings[6] = {
        "[ TRACE ]: ",
        "[ DEBUG ]: ",
        "[ INFO  ]: ",
        "[WARNING]: ",
        "[ ERROR ]: ",
        "[ FATAL ]: ",
    };

    char buffer[MAX_LOG_SIZE];
    memset(buffer, 0, sizeof(buffer));

    __builtin_va_list argPointer;
    va_start(argPointer, messageFormat);
    vsnprintf_s(buffer, MAX_LOG_SIZE, messageFormat, argPointer);
    va_end(argPointer);

    std::stringstream outStream;
    outStream << logLevelStrings[(size_t)level] << buffer << "\n";

    // Generic cross platform printing
    std::cout << outStream.str();
}

#endif