#include <log.h>
#include <iostream>

#ifdef _WIN32
#include <windows.h>
#include <wincon.h>

void logTrace(const char *message)
{
#ifdef LOG_LEVEL_TRACE
    const char *tag = "[ TRACE ]: ";
    HANDLE hConsole;
    hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    FlushConsoleInputBuffer(hConsole);
    SetConsoleTextAttribute(hConsole, FOREGROUND_BLUE);
    std::cout << tag << message << std::endl;
    SetConsoleTextAttribute(hConsole, 7);
#endif
}

void logDebug(const char *message)
{
#ifdef LOG_LEVEL_DEBUG
    const char *tag = "[ DEBUG ]: ";
    HANDLE hConsole;
    hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    FlushConsoleInputBuffer(hConsole);
    SetConsoleTextAttribute(hConsole, FOREGROUND_GREEN);
    std::cout << tag << message << std::endl;
    SetConsoleTextAttribute(hConsole, 7);
#endif
}

void logInfo(const char *message)
{
#ifdef LOG_LEVEL_INFO
    const char *tag = "[ INFO  ]: ";
    HANDLE hConsole;
    hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    FlushConsoleInputBuffer(hConsole);
    SetConsoleTextAttribute(hConsole, FOREGROUND_BLUE | FOREGROUND_GREEN);
    std::cout << tag << message << std::endl;
    SetConsoleTextAttribute(hConsole, 7);
#endif
}

void logWarning(const char *message)
{
#ifdef LOG_LEVEL_WARNING
    const char *tag = "[WARNING]: ";
    HANDLE hConsole;
    hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    FlushConsoleInputBuffer(hConsole);
    SetConsoleTextAttribute(hConsole, FOREGROUND_GREEN | FOREGROUND_RED);
    std::cout << tag << message << std::endl;
    SetConsoleTextAttribute(hConsole, 7);
#endif
}

void logError(const char *message)
{
#ifdef LOG_LEVEL_ERROR
    const char *tag = "[ ERROR ]: ";
    HANDLE hConsole;
    hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    FlushConsoleInputBuffer(hConsole);
    SetConsoleTextAttribute(hConsole, FOREGROUND_RED);
    std::cout << tag << message << std::endl;
    SetConsoleTextAttribute(hConsole, 7);
#endif
}

void logFatal(const char *message)
{
#ifdef LOG_LEVEL_FATAL
    const char *tag = "[ FATAL ]: ";
    HANDLE hConsole;
    hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    FlushConsoleInputBuffer(hConsole);
    SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_INTENSITY);
    std::cout << tag << message << std::endl;
    SetConsoleTextAttribute(hConsole, 7);
#endif
}

#elif
void logTrace(const char *message)
{
#ifdef LOG_LEVEL_TRACE
    const char *tag = "[ TRACE ]: ";
    std::cout << tag << message << std::endl;
#endif
}

void logDebug(const char *message)
{
#ifdef LOG_LEVEL_DEBUG

    const char *tag = "[ DEBUG ]: ";
    std::cout << tag << message << std::endl;
#endif
}

void logInfo(const char *message)
{
#ifdef LOG_LEVEL_INFO

    const char *tag = "[ INFO  ]: ";
    std::cout << tag << message << std::endl;
#endif
}

void logWarning(const char *message)
{
#ifdef LOG_LEVEL_WARNING

    const char *tag = "[WARNING]: ";
    std::cout << tag << message << std::endl;
#endif
}

void logError(const char *message)
{
#ifdef LOG_LEVEL_ERROR

    const char *tag = "[ ERROR ]: ";
    std::cout << tag << message << std::endl;
#endif
}

void logFatal(const char *message)
{
#ifdef LOG_LEVEL_FATAL

    const char *tag = "[ FATAL ]: ";
    std::cout << tag << message << std::endl;
#endif
}

#endif