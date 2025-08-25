#include "Console.h"
#include "../Core/Config.h"
#include "../Features/WeaponManager.h"
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
                DebugPrint("CheatManager and Console enabled successfully!");
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
        
        void Console::UpdateCheats(SDK::APlayerController* playerController) {
            if (!playerController) {
                return;
            }

            // Debug logging every 5 seconds
            static DWORD lastDebugTime = 0;
            DWORD currentTime = GetTickCount();
            bool shouldDebugLog = false && (currentTime - lastDebugTime) > 5000;

            // Get game objects
            auto myPawn = playerController->K2_GetPawn();
            auto playerPawn = static_cast<SDK::ARPlayerPawn*>(myPawn);
            auto world = Cheat::Config::GameState::g_pWorld;


            // Capture original speeds on first run (when character is available)
            if (playerPawn && playerPawn->GetRPawnMovementComponent() && !Cheat::Config::Features::OriginalSpeedsSaved) {
                auto moveComp = playerPawn->GetRPawnMovementComponent();
                Cheat::Config::Features::originalMovementSpeedModifier = moveComp->MovementSpeedModifier;
                Cheat::Config::Features::OriginalSpeedsSaved = true;
                
            }

            // =============================================================================
            // Ensure CheatManager exists - it's required for many cheats
            // =============================================================================
            if (!playerController->CheatManager) {
                static DWORD lastCheatManagerAttempt = 0;
                if (currentTime - lastCheatManagerAttempt > 1000) { // Try every second
                    if (Cheat::Utils::Console::EnableCheatManager(playerController)) {
                        LOG_INFO("CheatManager enabled successfully during update");
                    } else if (shouldDebugLog) {
                        LOG_INFO("CheatManager spawn attempt failed - will retry");
                    }
                    lastCheatManagerAttempt = currentTime;
                }
            }

            // Static state tracking for toggles
            static bool lastGodModeState = false;
            static bool lastSpeedHackState = false;

            // =============================================================================
            // DIRECT CHEAT IMPLEMENTATION (no CheatManager needed)
            // =============================================================================

            // Handle God Mode - direct health manipulation
            if (Cheat::Config::Features::GodMode && playerPawn && playerPawn->HealthComponent) {
                float maxHealth = playerPawn->HealthComponent->GetMaxHealth();
                playerPawn->HealthComponent->currentHealth = maxHealth;

                // Log state change
                if (Cheat::Config::Features::GodMode != lastGodModeState) {
                    LOG_INFO("God Mode enabled - direct health manipulation");
                    lastGodModeState = Cheat::Config::Features::GodMode;
                }
            } else if (!Cheat::Config::Features::GodMode && lastGodModeState) {
                LOG_INFO("God Mode disabled");
                lastGodModeState = Cheat::Config::Features::GodMode;
            }

            // Handle Speed Hack - direct character movement manipulation
            if (playerPawn && playerPawn->GetRPawnMovementComponent() && Cheat::Config::Features::OriginalSpeedsSaved) {
                if (Cheat::Config::Features::SpeedHack) {
                    auto moveComp = playerPawn->GetRPawnMovementComponent();
                    moveComp->MovementSpeedModifier = SDK::FRMutableFloat{ Cheat::Config::Features::SpeedMultiplier, Cheat::Config::Features::SpeedMultiplier, Cheat::Config::Features::SpeedMultiplier };

                    // Log state change
                    if (Cheat::Config::Features::SpeedHack != lastSpeedHackState) {
                        LOG_INFO("Speed Hack enabled - %.1fx speed (MovementSpeedModifier: %.1f)", Cheat::Config::Features::SpeedMultiplier, Cheat::Config::Features::originalMovementSpeedModifier);
                        lastSpeedHackState = Cheat::Config::Features::SpeedHack;
                    }
                } else {
                    // Reset to original speeds
                    auto moveComp = playerPawn->GetRPawnMovementComponent();
                    moveComp->MovementSpeedModifier = SDK::FRMutableFloat{ 1.0f, 1.0f, 1.0f };

                    // Log state change
                    if (lastSpeedHackState) {
                        LOG_INFO("Speed Hack disabled - reset to original speeds (MovementSpeedModifier: %.1f)", Cheat::Config::Features::originalMovementSpeedModifier);
                        lastSpeedHackState = Cheat::Config::Features::SpeedHack;
                    }
                }
            }

            // =============================================================================
            // WEAPON MODIFICATIONS
            // =============================================================================
            // Weapon modifications are now handled by WeaponManager::Update() for individual toggles

            // =============================================================================
            // CONTINUOUS CHEATS (applied every frame when enabled)
            // =============================================================================

            // Engine Rifle Heat Management is handled by WeaponManager::Update()
            // which is called from CheatMain::UpdateSubsystems()

            // Note: Aimbot is handled by AimbotController::Update()
            // which is also called from CheatMain::UpdateSubsystems()

            // Debug logging
            if (shouldDebugLog) {
                LOG_INFO("=== CHEAT STATUS DEBUG ===");
                LOG_INFO("God Mode: %s", Cheat::Config::Features::GodMode ? "ON" : "OFF");
                LOG_INFO("Speed Hack: %s", Cheat::Config::Features::SpeedHack ? "ON" : "OFF");
                LOG_INFO("Engine Rifle Heat Mgmt: %s", Cheat::Config::Features::EngineRifleHeatManagement ? "ON" : "OFF");
                LOG_INFO("Aimbot Enabled: %s ", Cheat::Config::Aimbot::enabled ? "ON" : "OFF");
                LOG_INFO("Aimbot Visibility Check: %s", Cheat::Config::Aimbot::visibilityCheck ? "ON" : "OFF");
                LOG_INFO("Aimbot FOV Radius: %.1f", Cheat::Config::Aimbot::fovRadius);
                LOG_INFO("Aimbot Max Distance: %.1f", Cheat::Config::Aimbot::maxDistance);
                LOG_INFO("CheatManager Available: %s", (playerController->CheatManager != nullptr) ? "YES" : "NO");
                lastDebugTime = currentTime;
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
