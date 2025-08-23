#pragma once

#include "../SDK/Engine_classes.hpp"
#include "../SDK/RGame_classes.hpp"
#include "../SDK/BP_EngineRifle_Script_classes.hpp"
#include <Windows.h>
#include <vector>

namespace Cheat {
    namespace Config {
        
        // =============================================================================
        // HOTKEY CONFIGURATION
        // =============================================================================
        namespace Hotkeys {
            // Feature toggles
            inline static BYTE WeaponMods = VK_F1;          // Apply weapon modifications
            inline static BYTE AimbotToggle = VK_F2;        // Toggle aimbot on/off
            inline static BYTE DumpBones = VK_F3;           // Dump enemy bones
            inline static BYTE ShowBoneDB = VK_F4;          // Display bone database
            
            // Aimbot controls
            inline static BYTE AimbotTrigger = VK_XBUTTON1;  // Mouse4 - Hold to aim
            
            // System controls
            inline static BYTE MenuToggle = VK_INSERT;       // Toggle ImGui menu
            inline static BYTE ExitCheat = VK_F9;           // Exit cheat system
        }
        
        // =============================================================================
        // GAME STATE GLOBALS
        // =============================================================================
        namespace GameState {
            // Core game objects
            inline static SDK::UEngine* g_pEngine = nullptr;
            inline static SDK::UWorld* g_pWorld = nullptr;
            inline static SDK::APlayerController* g_pMyController = nullptr;
            inline static SDK::APawn* g_pMyPawn = nullptr;
            inline static SDK::ACharacter* g_pMyCharacter = nullptr;
            
            // Targeting system
            inline static std::vector<SDK::AActor*> g_TargetsList{};
            inline static SDK::AActor* g_pCurrentTarget = nullptr;
            
            // Weapon system
            inline static SDK::ARPlayerPawn* g_pCachedPlayerPawn = nullptr;
            inline static SDK::ARWeapon* g_pCachedWeapon = nullptr;
            inline static SDK::URGWeaponScript* g_pCachedWeaponScript = nullptr;
            inline static SDK::UBP_EngineRifle_Script_C* g_pCachedEngineRifleScript = nullptr;
            inline static bool g_bIsEngineRifle = false;
        }
        
        // =============================================================================
        // AIMBOT CONFIGURATION
        // =============================================================================
        namespace Aimbot {
            // Core settings
            inline static bool enabled = true;
            inline static bool smoothEnabled = false;
            inline static bool visibilityCheck = false;
            inline static bool drawFOV = false;
            inline static bool predictiveAiming = false;
            inline static bool humanizeMovement = false;
            
            // Targeting parameters
            inline static float maxDistance = 50000.0f;
            inline static float fovRadius = 52800.0f;
            inline static float smoothFactor = 8.0f;
            inline static float maxTurnSpeed = 5180.0f;
            
            // Advanced settings
            inline static float reactionTime = 0.0f;
            inline static float targetSwitchDelay = 0.0f;
            inline static bool prioritizeHeadshots = false;
            inline static bool aimAtMovingTargets = true;
            inline static float maxAimSnapDistance = 1800.0f;
            inline static bool requireLineOfSight = true;
            
            // Aim zones (priority order)
            struct AimZones {
                bool head = true;
                bool chest = true;
                bool body = true;
            };
            inline static AimZones aimZones;
            
            // Visual settings
            struct Color {
                float r, g, b, a;
            };
            inline static Color fovColor = {1.0f, 1.0f, 1.0f, 0.3f};
            inline static Color enemyColor = {1.0f, 0.0f, 0.0f, 1.0f};
            inline static Color targetColor = {0.0f, 1.0f, 0.0f, 1.0f};
        }
        
        // =============================================================================
        // FEATURE FLAGS
        // =============================================================================
        namespace Features {
            inline static bool GodMode = true;              // Auto health restoration
            inline static bool WeaponMods = false;          // Weapon modifications applied
            inline static bool EngineRifleHeatManagement = true; // Prevent overheating
            inline static bool AutoCheatManager = true;     // Auto-enable cheat manager
        }
        
        // =============================================================================
        // SYSTEM STATE
        // =============================================================================
        namespace System {
            inline static bool Initialized = false;
            inline static bool ShouldExit = false;
            inline static DWORD LastFrameTime = 0;
            inline static DWORD LastPrintTime = 0;
        }
        
        // =============================================================================
        // CONFIGURATION FUNCTIONS
        // =============================================================================
        
        // Initialize all configuration values
        void Initialize();
        
        // Print current configuration to console
        void PrintConfiguration();
        
        // Update game state references
        void UpdateGameState();
        
        // Clear all game state references
        void ClearGameState();
        
    } // namespace Config
} // namespace Cheat
