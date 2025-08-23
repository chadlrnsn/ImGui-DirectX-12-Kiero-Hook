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
            inline static bool enabled = true;
            inline static bool smoothEnabled = false;
            inline static bool visibilityCheck = true;
            inline static bool drawFOV = false;

            
            // Targeting parameters
            inline static float maxDistance = 50000.0f;
            inline static float fovRadius = 52800.0f;
            inline static float smoothFactor = 8.0f;
            inline static float maxTurnSpeed = 5180.0f;
            
            // Advanced settings
            inline static float reactionTime = 0.0f;
            inline static float targetSwitchDelay = 0.0f;
            inline static bool aimAtMovingTargets = true;
            inline static float maxAimSnapDistance = 1800.0f;

            
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
