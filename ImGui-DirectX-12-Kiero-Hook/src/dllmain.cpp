#include <includes.h>
#include <d3d12hook.h>
#include <globals.h>


HANDLE g_consoleHandle = nullptr;
FILE* g_consoleStream = nullptr;

void CreateConsole();
void CleanupConsole();
void keyhandle();
void eraseHooks();
DWORD WINAPI MainThread(HMODULE hModule, LPVOID lpParam);
BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved);


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

void keyhandle() {
    while (!globals::g_break) {

        if (GetAsyncKeyState(VK_F9) & 1) {
            LOG_INFO("Uninjecting...");
            globals::g_break = true;
            break;
        }
    }
}

void eraseHooks()
{
    MH_DisableHook(MH_ALL_HOOKS);
    MH_RemoveHook(MH_ALL_HOOKS);
    MH_Uninitialize();
    LOG_INFO("All hooks erased and disabled minhook uninitialized");
}

DWORD WINAPI MainThread(HMODULE hModule, LPVOID lpParam) {

    LOG_INFO("Injected");

    std::thread KeyHandle(keyhandle);

    KeyHandle.detach();

    InitD3D12Hook();

    while (!globals::g_break) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    
    ReleaseD3D12Hook();
    
    eraseHooks();

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

        eraseHooks();
        LOG_INFO("Unhooked\n");
        CleanupConsole();
        break;
    }
    return TRUE;
}