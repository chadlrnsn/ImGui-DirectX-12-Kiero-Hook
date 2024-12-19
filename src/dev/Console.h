#pragma once
#include <windows.h>
#include <stdio.h>
#include <fstream>

void CreateConsole();
void CleanupConsole();

inline HANDLE g_consoleHandle = nullptr;
inline FILE *g_consoleStream = nullptr;

inline void CreateConsole()
{
    if (AllocConsole())
    {
        g_consoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);
        freopen_s(&g_consoleStream, "CONOUT$", "w", stdout);
        freopen_s(&g_consoleStream, "CONOUT$", "w", stderr);
        freopen_s(&g_consoleStream, "CONIN$", "r", stdin);
    }
}

inline void CleanupConsole()
{
    if (g_consoleStream)
    {
        fclose(g_consoleStream);
        FreeConsole();
        // Release console
        g_consoleHandle = nullptr;
        g_consoleStream = nullptr;
    }
}