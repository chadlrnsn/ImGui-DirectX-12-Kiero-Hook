#pragma once
#include <cstdarg>
#include <cstdio>

class Logger
{
public:
    enum class LogType
    {
        INFO,
        WARN,
        ERR,
        DEBUG,
    };

    static void log(LogType level, const char *format, ...)
    {
        va_list args;
        va_start(args, format);

        switch (level)
        {
        case LogType::INFO:
            printf("[INFO]\t");
            break;
        case LogType::WARN:
            printf("[WARN]\t");
            break;
        case LogType::ERR:
            printf("[ERROR]\t");
            break;
        case LogType::DEBUG:
            printf("[DEBUG]\t");
            break;
        }

        vprintf(format, args);
        printf("\n");

        va_end(args);
    }
};
#define LOG_ERROR(format, ...) Logger::log(Logger::LogType::ERR, format, __VA_ARGS__)
#define LOG_WARN(format, ...) Logger::log(Logger::LogType::WARN, format, __VA_ARGS__)
#define LOG_INFO(format, ...) Logger::log(Logger::LogType::INFO, format, __VA_ARGS__)
#define LOG_DEBUG(format, ...) Logger::log(Logger::LogType::DEBUG, format, __VA_ARGS__)
