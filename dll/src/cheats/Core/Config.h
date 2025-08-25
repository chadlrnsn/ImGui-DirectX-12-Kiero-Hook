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
            inline static BYTE LogWeaponStats = VK_F5;      // Log all weapon stats for debugging

            // Aimbot controls (configurable)
            extern BYTE AimbotTrigger;                       // Hold to aim (configurable, default: Mouse4)

            // System controls
            inline static BYTE MenuToggle = VK_INSERT;       // Toggle ImGui menu
            inline static BYTE ExitCheat = VK_F9;           // Exit cheat system

            // Hotkey configuration state
            extern bool IsCapturingHotkey;                   // True when waiting for user to press a key
            extern BYTE* CurrentHotkeyBeingSet;              // Pointer to the hotkey variable being configured
        }

        // =============================================================================
        // GUI CONFIGURATION
        // =============================================================================
        namespace GUI {
            extern float Scale;                              // GUI scale multiplier (1.0 = normal, 2.0 = double size)
        }
        
        // =============================================================================
        // RUNTIME STATE (targeting-only, moved core object pointers to Services::GameServices)
        // =============================================================================
        namespace GameState {
            // Targeting system (kept in Config)
            extern std::vector<SDK::AActor*> g_TargetsList;
            extern SDK::AActor* g_pCurrentTarget;
            extern TargetInfo g_CurrentTargetInfo;
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
            extern float maxTurnSpeed;

            // Advanced settings
            extern float reactionTime;
            extern float targetSwitchDelay;
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
            extern bool EngineRifleHeatManagement; // Prevent overheating
            extern bool AutoCheatManager;     // Auto-enable cheat manager

            // Speed hack configuration
            extern float SpeedMultiplier;    // Speed multiplier (1.0 = normal, 2.0 = 2x, etc.)
            extern  SDK::FRMutableFloat originalMovementSpeedModifier; // Original movement speed modifier (overwritten at startup)
            extern bool OriginalSpeedsSaved;      // Whether original speeds have been captured

            // Movement hack configuration
            extern bool SlowImmunity;        // Toggle slow immunity
            extern bool JumpHeightHack;      // Enable jump height modification
            extern bool DashSpeedHack;       // Enable dash speed modification

            // Movement multipliers
            extern float JumpHeightMultiplier;   // Jump height multiplier (1.0 = normal, 2.0 = 2x, etc.)
            extern float DashSpeedMultiplier;    // Dash speed multiplier (1.0 = normal, 2.0 = 2x, etc.)

            // Original movement values for restoration
            extern float originalJumpHeight;     // Original jump height value
            extern float originalDashSpeed;      // Original dash speed value
            extern float originalDashTime;       // Original dash time value
            extern bool originalSlowImmunity;    // Original slow immunity state
            extern bool OriginalMovementValuesSaved; // Whether original movement values have been captured

            // Individual weapon modification flags
            extern bool InfiniteAmmo;        // No ammo cost
            extern bool IncreasedDamage;     // Massively increased damage
            extern bool HighCritMultiplier;  // Extremely high critical hit multiplier
            extern bool FastRateOfFire;      // Super fast rate of fire
            extern bool NoCooldown;          // No cooldown
            extern bool NoRecoil;            // No recoil, instant recovery, and perfect accuracy
            extern bool InstantReload;       // Instant reload

            // New: multipliers/overrides (applied when corresponding toggles are enabled)
            extern float DamageMultiplier;     // Multiplies BaseWeaponDamage when IncreasedDamage is ON
            extern float CritMultiplier;       // Multiplies BaseWeaponCriticalMultiplier when HighCritMultiplier is ON

            // New: rate of fire override (slider from Min..Max; applied when enabled)
            extern bool RateOfFireOverride;    // Use RateOfFireValue instead of original/fast toggle
            extern float RateOfFireValue;      // Desired rate of fire within MinMaxRange
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
