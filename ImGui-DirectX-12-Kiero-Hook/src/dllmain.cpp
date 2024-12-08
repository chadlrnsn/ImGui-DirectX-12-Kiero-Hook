#include <includes.h>
#include <d3d12hook.h>
#include <globals.h>

HANDLE g_consoleHandle = nullptr;
FILE *g_consoleStream = nullptr;

// Добавляем флаг для отслеживания состояния выгрузки
static bool g_cleanup_done = false;

void CreateConsole();
void CleanupConsole();
void keyhandle();
void RemoveAllHooks();

DWORD WINAPI MainThread(HMODULE hModule, LPVOID lpParam);
BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved);

void CreateConsole()
{
    if (AllocConsole())
    {
        g_consoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);
        freopen_s(&g_consoleStream, "CONOUT$", "w", stdout);
        freopen_s(&g_consoleStream, "CONOUT$", "w", stderr);
        freopen_s(&g_consoleStream, "CONIN$", "r", stdin);
    }
}

void CleanupConsole()
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

void keyhandle()
{
    while (!globals::g_break)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        if (GetAsyncKeyState(VK_F9) & 1)
        {
            LOG_INFO("Uninjecting...");
            globals::g_break = true;
            break;
        }
    }
}

void RemoveAllHooks()
{
    MH_DisableHook(MH_ALL_HOOKS);
    MH_RemoveHook(MH_ALL_HOOKS);
    MH_Uninitialize();
    LOG_INFO("All hooks erased and disabled minhook uninitialized");
}

void Cleanup()
{
    if (!g_cleanup_done)
    {
        LOG_INFO("Starting cleanup...");
        ReleaseD3D12Hook();
        RemoveAllHooks();
        CleanupConsole();
        g_cleanup_done = true;
        LOG_INFO("Cleanup complete");
    }
}

DWORD WINAPI MainThread(HMODULE hModule, LPVOID lpParam)
{
    LOG_INFO("Injected");

    std::thread KeyHandle(keyhandle);
    KeyHandle.detach();

    InitD3D12Hook();

    while (!globals::g_break)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    LOG_INFO("Starting DLL unload sequence...");
    Cleanup();

    LOG_INFO("Unloading DLL...");
    FreeLibraryAndExitThread(hModule, 0);
    return 0;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        DisableThreadLibraryCalls(hModule);
        CreateConsole();
        CreateThread(nullptr, 0, (LPTHREAD_START_ROUTINE)MainThread, hModule, 0, nullptr);
        break;

    case DLL_PROCESS_DETACH:
        // Выполняем очистку только если она еще не была выполнена
        Cleanup();
        break;
    }
    return TRUE;
}