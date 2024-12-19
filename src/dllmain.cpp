#include <includes.h>
#include <hooks/d3d12hook.h>
#include <dev/Console.h>
#include <dev/logger.h>

static bool g_cleanup_done = false;

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

DWORD WINAPI MainThread(LPVOID lpParam)
{
    LOG_INFO("Injected");
    InitD3D12Hook();

    while (true)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));

        if (GetAsyncKeyState(VK_F9) & 0x8000)
            break;
    }

    LOG_INFO("Uninjecting...");
    Cleanup();

    LOG_INFO("Unloading DLL...");
    FreeLibraryAndExitThread((HMODULE)lpParam, 0);
    return 0;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        DisableThreadLibraryCalls(hModule);
        CreateConsole();
        CloseHandle(CreateThread(nullptr, 0, MainThread, hModule, 0, nullptr));
        break;

    case DLL_PROCESS_DETACH:
        // Выполняем очистку только если она еще не была выполнена
        Cleanup();
        break;
    }
    return TRUE;
}