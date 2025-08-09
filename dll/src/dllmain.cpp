#include <includes.h>
#include <hooks/d3d12hook.h>
#include <dev/Console.h>
#include <dev/logger.h>

namespace Hook
{
    static bool g_cleanup_done = false;
    static Console g_console; // RAII object

    void RemoveAllHooks()
    {
        MH_DisableHook(MH_ALL_HOOKS);
        MH_RemoveHook(MH_ALL_HOOKS);
        MH_Uninitialize();
        LOG_INFO("All hooks removed and MinHook uninitialized");
    }

    void Cleanup()
    {
        if (!g_cleanup_done)
        {
            LOG_INFO("Starting cleanup...");

            // Disable hooks
            RemoveAllHooks();
            std::this_thread::sleep_for(std::chrono::milliseconds(100));

            // Release DirectX resources
            ReleaseD3D12Hook();

            g_cleanup_done = true;
            LOG_INFO("Cleanup complete");
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    }

    bool Initialize()
    {
        LOG_INFO("Initializing hooks...");

        if (!InitD3D12Hook())
        {
            LOG_ERROR("Failed to initialize DirectX 12 hook");
            return false;
        }

        LOG_INFO("Hooks initialized successfully");
        return true;
    }
}

DWORD WINAPI MainThread(HMODULE hModule, LPVOID)
{
    // Инициализация
    LOG_INFO("DLL injected successfully");

    if (!Hook::Initialize())
    {
        Hook::Cleanup();
        FreeLibraryAndExitThread(hModule, 1);
        return 1;
    }

    while (true)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));

        if (GetAsyncKeyState(VK_F9) & 0x8000)
            break;
    }

    LOG_INFO("Starting unload sequence...");
    Hook::Cleanup();

    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    LOG_INFO("Unloading DLL...");

    FreeLibraryAndExitThread(hModule, 0);
    return 0;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD reason, LPVOID lpReserved)
{
    switch (reason)
    {
    case DLL_PROCESS_ATTACH:
        DisableThreadLibraryCalls(hModule);
        CloseHandle(CreateThread(0, 0, (LPTHREAD_START_ROUTINE)MainThread, hModule, 0, 0));
        break;

    case DLL_PROCESS_DETACH:
        Hook::Cleanup();
        break;
    }
    return TRUE;
}