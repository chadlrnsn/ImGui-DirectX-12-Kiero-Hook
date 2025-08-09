#pragma once
#include <cstdarg>
#include <cstdio>
#include <fmt/base.h>
#include <fmt/color.h>

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
            fmt::print(fg(fmt::color::green), "[INFO]\t");
            break;
        case LogType::WARN:
            fmt::print(fg(fmt::color::yellow), "[WARN]\t");
            break;
        case LogType::ERR:
            fmt::print(fg(fmt::color::red), "[ERROR]\t");
            break;
        case LogType::DEBUG:
            fmt::print(fg(fmt::color::blue), "[DEBUG]\t");
            break;
        }

        vprintf(format, args);
        fmt::print("\n");

        va_end(args);
    }
};

#define LOG_ERROR(format, ...) Logger::log(Logger::LogType::ERR, format, __VA_ARGS__)
#define LOG_WARN(format, ...) Logger::log(Logger::LogType::WARN, format, __VA_ARGS__)
#define LOG_INFO(format, ...) Logger::log(Logger::LogType::INFO, format, __VA_ARGS__)
#define LOG_DEBUG(format, ...) Logger::log(Logger::LogType::DEBUG, format, __VA_ARGS__)
