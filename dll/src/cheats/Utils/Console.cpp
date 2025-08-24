#include "Console.h"
#include "../Core/Config.h"
#include <iostream>
#include <dev/logger.h>

namespace Cheat {
    namespace Utils {
        
        bool Console::s_autoCheatManager = true;
        
        void Console::Write(const std::string& text) {
            std::cout << text << std::endl;
        }
        
        void Console::Write(const std::wstring& text) {
            std::wcout << text << std::endl;
        }
        
        void Console::DebugPrint(const std::string& text) {
            Write(text);
        }
        
        bool Console::EnableCheatManager(SDK::APlayerController* playerController) {
            if (!playerController) {
                LOG_ERROR("PlayerController is null");
                return false;
            }
            
            auto engine = SDK::UEngine::GetEngine();
            if (!engine) {
                LOG_ERROR("Engine not found");
                return false;
            }
            
            bool cheatSpawned = false;
            bool consoleSpawned = false;
            
            // Spawn CheatManager for the PlayerController
            if (!playerController->CheatManager && playerController->CheatClass) {
                if (auto cheatObject = SDK::UGameplayStatics::SpawnObject(playerController->CheatClass, playerController)) {
                    playerController->CheatManager = static_cast<SDK::UCheatManager*>(cheatObject);
                    if (playerController->CheatManager) {
                        playerController->EnableCheats();
                        LOG_INFO("CheatManager spawned: %s", playerController->CheatManager->GetFullName());
                        cheatSpawned = true;
                    }
                }
            } else if (playerController->CheatManager) {
                cheatSpawned = true;
            }
            
            // Spawn Console for the GameViewport
            if (engine->ConsoleClass && engine->GameViewport && !engine->GameViewport->ViewportConsole) {
                if (auto consoleObject = SDK::UGameplayStatics::SpawnObject(engine->ConsoleClass, engine->GameViewport)) {
                    if (auto consoleInstance = static_cast<SDK::UConsole*>(consoleObject)) {
                        engine->GameViewport->ViewportConsole = consoleInstance;
                        LOG_INFO("Console spawned: %s", consoleInstance->GetFullName());
                        consoleSpawned = true;
                    }
                }
            } else if (engine->GameViewport->ViewportConsole) {
                consoleSpawned = true;
            }
            
            if (cheatSpawned && consoleSpawned) {
                Cheat::Config::Features::AutoCheatManager = false;
                DebugPrint("Enabling cheat commands!");
                playerController->CheatManager->God();
                playerController->CheatManager->Slomo(2);
                return true;
            } else {
                if (!cheatSpawned) {
                    DebugPrint("Failed to spawn CheatManager!");
                }
                if (!consoleSpawned) {
                    DebugPrint("Failed to spawn ViewPort Console!");
                }
                Cheat::Config::Features::AutoCheatManager = true;
                return false;
            }
        }
        
        void Console::PrintControls() {
            LOG_INFO("Controls:");
            LOG_INFO("- F1: Apply weapon modifications");
            LOG_INFO("- F2: Toggle aimbot on/off");
            LOG_INFO("- F3: Dump enemy bones");
            LOG_INFO("- F4: Display bone database status");
            LOG_INFO("- Mouse4: Hold to activate aimbot");
            LOG_INFO("- Insert: Toggle ImGui menu");
            LOG_INFO("- F9: Exit cheat system");
        }
        
    } // namespace Utils
} // namespace Cheat
