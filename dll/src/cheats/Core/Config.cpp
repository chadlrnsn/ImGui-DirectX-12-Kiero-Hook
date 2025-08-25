#include "Config.h"
#include <dev/logger.h>
#include <iostream>


namespace Cheat {
    namespace Config {
        namespace GameState {
            // Define globals exactly once
            SDK::UEngine* g_pEngine = nullptr;
            SDK::UWorld* g_pWorld = nullptr;
            SDK::APlayerController* g_pMyController = nullptr;
            SDK::APawn* g_pMyPawn = nullptr;

            std::vector<SDK::AActor*> g_TargetsList{};
            SDK::AActor* g_pCurrentTarget = nullptr;
            TargetInfo g_CurrentTargetInfo{};

            SDK::ARPlayerPawn* g_pCachedPlayerPawn = nullptr;
            SDK::ARWeapon* g_pCachedWeapon = nullptr;
            SDK::URGWeaponScript* g_pCachedWeaponScript = nullptr;
            SDK::UBP_EngineRifle_Script_C* g_pCachedEngineRifleScript = nullptr;
            bool g_bIsEngineRifle = false;
        }

        // =============================================================================
        // HOTKEY CONFIGURATION - Define configurable hotkeys
        // =============================================================================
        namespace Hotkeys {
            BYTE AimbotTrigger = VK_XBUTTON1;               // Default: Mouse4
            bool IsCapturingHotkey = false;
            BYTE* CurrentHotkeyBeingSet = nullptr;
        }

        // =============================================================================
        // GUI CONFIGURATION - Define GUI settings
        // =============================================================================
        namespace GUI {
            float Scale = 1.5f;                             // Default: 1.5x scale for better visibility on high-DPI displays
        }

        // =============================================================================
        // AIMBOT CONFIGURATION - Define once here
        // =============================================================================
        namespace Aimbot {
            // Core settings
            bool enabled = true;
            bool smoothEnabled = true;
            bool visibilityCheck = true;
            bool drawFOV = false;

            // Targeting parameters
            float maxDistance = 50000.0f;
            float fovRadius = 52800.0f;
            float maxTurnSpeed = 1000.0f;

            // Advanced settings
            float reactionTime = 0.0f;
            float targetSwitchDelay = 0.0f;
            float maxAimSnapDistance = 1800.0f;

            // Aim zones
            AimZones aimZones;

            // Visual settings
            Color fovColor = {1.0f, 1.0f, 1.0f, 0.3f};
            Color enemyColor = {1.0f, 0.0f, 0.0f, 1.0f};
            Color targetColor = {0.0f, 1.0f, 0.0f, 1.0f};
        }

        // =============================================================================
        // FEATURE FLAGS - Define once here
        // =============================================================================
        namespace Features {
            bool GodMode = true;             // CheatManager->God() invincibility
            bool SpeedHack = false;           // CheatManager->Slomo(2) speed boost
            bool EngineRifleHeatManagement = false; // Prevent overheating
            bool AutoCheatManager = true;     // Auto-enable cheat manager

            // Speed hack configuration
            float SpeedMultiplier = 2.0f;    // Default 2x speed
 SDK::FRMutableFloat originalMovementSpeedModifier = SDK::FRMutableFloat{ 1.0f, 1.0f, 1.0f }; // Original movement speed modifier (overwritten at startup)
            bool OriginalSpeedsSaved = false;      // Whether original speeds have been captured

            // Movement hack configuration
            bool SlowImmunity = false;        // Slow immunity disabled by default
            bool JumpHeightHack = false;      // Jump height hack disabled by default
            bool DashSpeedHack = false;       // Dash speed hack disabled by default

            // Movement multipliers
            float JumpHeightMultiplier = 2.0f;   // Default 2x jump height
            float DashSpeedMultiplier = 2.0f;    // Default 2x dash speed

            // Original movement values for restoration
            float originalJumpHeight = 0.0f;     // Will be captured at runtime
            float originalDashSpeed = 0.0f;      // Will be captured at runtime
            float originalDashTime = 0.0f;       // Will be captured at runtime
            bool originalSlowImmunity = false;   // Will be captured at runtime
            bool OriginalMovementValuesSaved = false; // Whether original movement values have been captured

            // Individual weapon modification flags
            bool InfiniteAmmo = false;        // No ammo cost
            bool IncreasedDamage = false;     // Massively increased damage
            bool HighCritMultiplier = false;  // Extremely high critical hit multiplier
            bool FastRateOfFire = false;      // Super fast rate of fire
            bool NoCooldown = false;          // No cooldown
            bool NoRecoil = false;            // No recoil, instant recovery, and perfect accuracy
            bool InstantReload = false;       // Instant reload

            // New: multipliers/overrides
            float DamageMultiplier = 2.0f;     // 2x damage by default
            float CritMultiplier = 2.0f;       // 2x crit multiplier by default

            bool RateOfFireOverride = false;   // off by default; use FastRateOfFire instead
            float RateOfFireValue = 0.0f;      // will be clamped between Min..Max at apply-time
        }

        // =============================================================================
        // DEBUG SETTINGS - Define once here
        // =============================================================================
        namespace Debug {
            bool enableMathLogging = false;   // Enable detailed math function logging
        }
    }
}

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
            LOG_INFO("- F2: Toggle aimbot on/off");
            LOG_INFO("- F3: Dump enemy bones");
            LOG_INFO("- F4: Display bone database");
            LOG_INFO("- F5: Log weapon stats (debug)");
            LOG_INFO("- Insert: Toggle menu");
            LOG_INFO("- F9: Unload cheat (causes crash sometimes, might fix eventually)");

            // Aimbot settings
            LOG_INFO("Aimbot Configuration:");
            LOG_INFO("- Status: %s", Aimbot::enabled ? "ENABLED" : "DISABLED");
            LOG_INFO("- Max Distance: %.0f units", Aimbot::maxDistance);
            LOG_INFO("- FOV Radius: %.0f degrees", Aimbot::fovRadius);
            LOG_INFO("- Visibility Check: %s", Aimbot::visibilityCheck ? "Enabled" : "Disabled");
            LOG_INFO("- Max Turn Speed: %.0f deg/s", Aimbot::maxTurnSpeed);
            LOG_INFO("- Reaction Time: %.2fs", Aimbot::reactionTime);

            // Feature flags
            LOG_INFO("Features:");
            LOG_INFO("- God Mode: %s", Features::GodMode ? "Enabled" : "Disabled");
            LOG_INFO("- Engine Rifle Heat Management: %s", Features::EngineRifleHeatManagement ? "Enabled" : "Disabled");
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
