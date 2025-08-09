#include <framework/stdafx.h>
#include "Console.h"

Console::Console() : m_consoleHandle(nullptr), m_consoleStream(nullptr)
{
    CreateConsole();
}

Console::~Console()
{
    CleanupConsole();
}

Console::Console(Console&& other) noexcept 
    : m_consoleHandle(other.m_consoleHandle), m_consoleStream(other.m_consoleStream)
{
    other.m_consoleHandle = nullptr;
    other.m_consoleStream = nullptr;
}

Console& Console::operator=(Console&& other) noexcept
{
    if (this != &other)
    {
        CleanupConsole();
        m_consoleHandle = other.m_consoleHandle;
        m_consoleStream = other.m_consoleStream;
        other.m_consoleHandle = nullptr;
        other.m_consoleStream = nullptr;
    }
    return *this;
}

std::unique_ptr<Console> Console::Create()
{
    return std::unique_ptr<Console>(new Console());
}

void Console::CreateConsole()
{
    if (AllocConsole())
    {
        m_consoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);
        freopen_s(&m_consoleStream, "CONOUT$", "w", stdout);
        freopen_s(&m_consoleStream, "CONOUT$", "w", stderr);
        freopen_s(&m_consoleStream, "CONIN$", "r", stdin);

        // enable virtual terminal processing
        if (m_consoleHandle != INVALID_HANDLE_VALUE)
        {
            DWORD dwMode = 0;
            if (GetConsoleMode(m_consoleHandle, &dwMode))
            {
                dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
                SetConsoleMode(m_consoleHandle, dwMode);
            }
        }
    }
}

void Console::CleanupConsole()
{
    if (m_consoleStream)
    {
        fclose(m_consoleStream);
        FreeConsole();
        m_consoleHandle = nullptr;
        m_consoleStream = nullptr;
    }
}






