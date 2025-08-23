#include "Config.h"
#include <dev/logger.h>
#include <iostream>

namespace Cheat {
    namespace Config {
        
        void Initialize() {
            LOG_INFO("Initializing cheat configuration...");
            
            // Initialize system state
            System::Initialized = false;
            System::ShouldExit = false;
            System::LastFrameTime = GetTickCount();
            System::LastPrintTime = GetTickCount();
            
            // Clear game state
            ClearGameState();
            
            LOG_INFO("Configuration initialized successfully");
        }
        
        void PrintConfiguration() {
            LOG_INFO("=== CHEAT SYSTEM CONFIGURATION ===");
            
            // Hotkeys
            LOG_INFO("Hotkeys:");
            LOG_INFO("- F1: Apply weapon modifications");
            LOG_INFO("- F2: Toggle aimbot on/off");
            LOG_INFO("- F3: Dump enemy bones");
            LOG_INFO("- F4: Display bone database");
            LOG_INFO("- Mouse4: Hold to activate aimbot");
            LOG_INFO("- Insert: Toggle ImGui menu");
            LOG_INFO("- F9: Exit cheat system");
            
            // Aimbot settings
            LOG_INFO("Aimbot Configuration:");
            LOG_INFO("- Status: %s", Aimbot::enabled ? "ENABLED" : "DISABLED");
            LOG_INFO("- Max Distance: %.0f units", Aimbot::maxDistance);
            LOG_INFO("- FOV Radius: %.0f degrees", Aimbot::fovRadius);
            LOG_INFO("- Smooth Aiming: %s", Aimbot::smoothEnabled ? "Enabled" : "Disabled");
            LOG_INFO("- Humanized Movement: %s", Aimbot::humanizeMovement ? "Enabled" : "Disabled");
            LOG_INFO("- Visibility Check: %s", Aimbot::visibilityCheck ? "Enabled" : "Disabled");
            LOG_INFO("- Max Turn Speed: %.0f deg/s", Aimbot::maxTurnSpeed);
            LOG_INFO("- Reaction Time: %.2fs", Aimbot::reactionTime);
            
            // Feature flags
            LOG_INFO("Features:");
            LOG_INFO("- God Mode: %s", Features::GodMode ? "Enabled" : "Disabled");
            LOG_INFO("- Engine Rifle Heat Management: %s", Features::EngineRifleHeatManagement ? "Enabled" : "Disabled");
            LOG_INFO("- Auto Cheat Manager: %s", Features::AutoCheatManager ? "Enabled" : "Disabled");
        }
        
        void UpdateGameState() {
            // Update world reference
            GameState::g_pWorld = SDK::UWorld::GetWorld();
            if (!GameState::g_pWorld) {
                return;
            }
            
            // Update engine reference
            GameState::g_pEngine = SDK::UEngine::GetEngine();
            
            // Update game instance and player controller
            if (GameState::g_pWorld->OwningGameInstance && 
                GameState::g_pWorld->OwningGameInstance->LocalPlayers.Num() > 0) {
                
                GameState::g_pMyController = GameState::g_pWorld->OwningGameInstance->LocalPlayers[0]->PlayerController;
                
                if (GameState::g_pMyController) {
                    // Update pawn references
                    GameState::g_pMyPawn = GameState::g_pMyController->K2_GetPawn();
                    GameState::g_pMyCharacter = static_cast<SDK::ACharacter*>(GameState::g_pMyPawn);
                    
                    // Update player pawn for weapon system
                    GameState::g_pCachedPlayerPawn = static_cast<SDK::ARPlayerPawn*>(GameState::g_pMyPawn);
                }
            }
        }
        
        void ClearGameState() {
            // Clear core game objects
            GameState::g_pEngine = nullptr;
            GameState::g_pWorld = nullptr;
            GameState::g_pMyController = nullptr;
            GameState::g_pMyPawn = nullptr;
            GameState::g_pMyCharacter = nullptr;
            
            // Clear targeting system
            GameState::g_TargetsList.clear();
            GameState::g_pCurrentTarget = nullptr;
            
            // Clear weapon system
            GameState::g_pCachedPlayerPawn = nullptr;
            GameState::g_pCachedWeapon = nullptr;
            GameState::g_pCachedWeaponScript = nullptr;
            GameState::g_pCachedEngineRifleScript = nullptr;
            GameState::g_bIsEngineRifle = false;
        }
        
    } // namespace Config
} // namespace Cheat
