#pragma once

#include "../SDK/Engine_classes.hpp"
#include "../SDK/RGame_classes.hpp"
#include "../SDK/BP_EngineRifle_Script_classes.hpp"
#include <Windows.h>
#include <vector>
#include <string>

// Forward declaration for TargetInfo
struct TargetInfo {
    SDK::AActor* actor = nullptr;
    SDK::FVector position;
    float distance = 0.0f;
    float fovDistance = 0.0f;
    bool isVisible = false;
    SDK::FVector aimPoint;

    // Bone targeting information
    int targetBoneIndex = -1;
    std::string targetBoneName;
    bool hasBoneTarget = false;
};

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
        // GAME STATE GLOBALS (single-definition via extern)
        // =============================================================================
        namespace GameState {
            // Core game objects
            extern SDK::UEngine* g_pEngine;
            extern SDK::UWorld* g_pWorld;
            extern SDK::APlayerController* g_pMyController;
            extern SDK::APawn* g_pMyPawn;
            extern SDK::ACharacter* g_pMyCharacter;

            // Targeting system
            extern std::vector<SDK::AActor*> g_TargetsList;
            extern SDK::AActor* g_pCurrentTarget;
            extern TargetInfo g_CurrentTargetInfo;

            // Weapon system
            extern SDK::ARPlayerPawn* g_pCachedPlayerPawn;
            extern SDK::ARWeapon* g_pCachedWeapon;
            extern SDK::URGWeaponScript* g_pCachedWeaponScript;
            extern SDK::UBP_EngineRifle_Script_C* g_pCachedEngineRifleScript;
            extern bool g_bIsEngineRifle;
        }
        
        // =============================================================================
        // AIMBOT CONFIGURATION
        // =============================================================================
        namespace Aimbot {
            // Core settings
            extern bool enabled;
            extern bool smoothEnabled;
            extern bool visibilityCheck;
            extern bool drawFOV;


            // Targeting parameters
            extern float maxDistance;
            extern float fovRadius;
            extern float smoothFactor;
            extern float maxTurnSpeed;

            // Advanced settings
            extern float reactionTime;
            extern float targetSwitchDelay;
            extern bool aimAtMovingTargets;
            extern float maxAimSnapDistance;


            // Aim zones (priority order)
            struct AimZones {
                bool head = true;
                bool chest = true;
                bool body = true;
            };
            extern AimZones aimZones;

            // Visual settings
            struct Color {
                float r, g, b, a;
            };
            extern Color fovColor;
            extern Color enemyColor;
            extern Color targetColor;
        }
        
        // =============================================================================
        // DEBUG SETTINGS
        // =============================================================================
        namespace Debug {
            extern bool enableMathLogging;   // Enable detailed math function logging
        }

        // =============================================================================
        // FEATURE FLAGS
        // =============================================================================
        namespace Features {
            extern bool GodMode;             // CheatManager->God() invincibility
            extern bool SpeedHack;           // CheatManager->Slomo(2) speed boost
            extern bool WeaponMods;          // Weapon modifications applied
            extern bool EngineRifleHeatManagement; // Prevent overheating
            extern bool AutoCheatManager;     // Auto-enable cheat manager

            // Speed hack configuration
            extern float SpeedMultiplier;    // Speed multiplier (1.0 = normal, 2.0 = 2x, etc.)
            extern float OriginalMaxWalkSpeed;     // Original MaxWalkSpeed (saved at startup)
            extern float OriginalMaxAcceleration; // Original MaxAcceleration (saved at startup)
            extern bool OriginalSpeedsSaved;      // Whether original speeds have been captured
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
