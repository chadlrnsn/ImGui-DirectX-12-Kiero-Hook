﻿#include <includes.h>
#include <d3d12hook.h>

HANDLE g_consoleHandle = nullptr;
FILE* g_consoleStream = nullptr;

void CreateConsole() {
    if (AllocConsole()) {
        g_consoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);
        freopen_s(&g_consoleStream, "CONOUT$", "w", stdout);
        freopen_s(&g_consoleStream, "CONOUT$", "w", stderr);
        freopen_s(&g_consoleStream, "CONIN$", "r", stdin);
    }
}

void CleanupConsole() {
    if (g_consoleStream) {
        fclose(g_consoleStream);
        g_consoleStream = nullptr;
    }
    if (g_consoleHandle) {
        FreeConsole();
        g_consoleHandle = nullptr;
    }
}

DWORD WINAPI MainThread(HMODULE hModule, LPVOID lpParam) {

    std::cout << "Injected" << std::endl;

    InitD3D12Hook();

    while (true) {

        if (GetAsyncKeyState(VK_F9) & 1) {
            std::cout << "Uninjecting..." << std::endl;
            break;
        }
    }
    
    ReleaseD3D12Hook();
    
    MH_DisableHook(MH_ALL_HOOKS);
    MH_RemoveHook(MH_ALL_HOOKS);
    MH_Uninitialize();

    FreeLibraryAndExitThread(hModule, 0);
    return 0;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {
    switch (ul_reason_for_call) {
    case DLL_PROCESS_ATTACH:
        DisableThreadLibraryCalls(hModule);
        CreateConsole();
        CreateThread(nullptr, 0, (LPTHREAD_START_ROUTINE)MainThread, hModule, 0, nullptr);
        break;

    case DLL_PROCESS_DETACH:
        MH_DisableHook(MH_ALL_HOOKS);
        MH_RemoveHook(MH_ALL_HOOKS);
        MH_Uninitialize();

        printf("Unhooked\n");
        CleanupConsole();
        break;
    }
    return TRUE;
}