#include "PlayerEffectsService.h"
#include "../Utils/Console.h"
#include <dev/logger.h>

namespace Cheat {
namespace Services {

void PlayerEffectsService::Update(SDK::APlayerController* playerController) {
    if (!playerController) return;

    // Debug logging cadence reused
    static DWORD lastDebugTime = 0;
    DWORD currentTime = GetTickCount();
    bool shouldDebugLog = false && (currentTime - lastDebugTime) > 5000;

    // Ensure CheatManager exists (spawn attempts at most once per second)
    if (!playerController->CheatManager) {
        static DWORD lastCheatManagerAttempt = 0;
        if (currentTime - lastCheatManagerAttempt > 1000) {
            if (Cheat::Utils::Console::EnableCheatManager(playerController)) {
                LOG_INFO("CheatManager enabled successfully during update");
            }
            lastCheatManagerAttempt = currentTime;
        }
    }

    // Get pawn and movement component
    auto myPawn = playerController->K2_GetPawn();
    auto playerPawn = static_cast<SDK::ARPlayerPawn*>(myPawn);

    // Capture originals if needed
    if (playerPawn && playerPawn->GetRPawnMovementComponent() && !Cheat::Config::Features::OriginalSpeedsSaved) {
        auto moveComp = playerPawn->GetRPawnMovementComponent();
        Cheat::Config::Features::originalMovementSpeedModifier = moveComp->MovementSpeedModifier;
        Cheat::Config::Features::OriginalSpeedsSaved = true;
    }
    if (playerPawn && playerPawn->GetRPawnMovementComponent() && !Cheat::Config::Features::OriginalMovementValuesSaved) {
        auto moveComp = playerPawn->GetRPawnMovementComponent();
        Cheat::Config::Features::originalJumpHeight = moveComp->JumpHeight;
        Cheat::Config::Features::originalDashSpeed = moveComp->DashSpeed;
        Cheat::Config::Features::originalDashTime = moveComp->DashTime;
        Cheat::Config::Features::originalSlowImmunity = moveComp->bSlowImmunity;
        Cheat::Config::Features::OriginalMovementValuesSaved = true;
        LOG_INFO("Original movement values captured: Jump=%.1f, Dash=%.1f/%.1f, SlowImmunity=%s",
            Cheat::Config::Features::originalJumpHeight,
            Cheat::Config::Features::originalDashSpeed,
            Cheat::Config::Features::originalDashTime,
            Cheat::Config::Features::originalSlowImmunity ? "true" : "false");
    }

    // Track last states
    static bool lastGodModeState = false;
    static bool lastSpeedHackState = false;
    static bool lastSlowImmunityState = false;
    static bool lastJumpHeightHackState = false;
    static bool lastDashSpeedHackState = false;

    // God Mode
    if (Cheat::Config::Features::GodMode && playerPawn && playerPawn->HealthComponent) {
        float maxHealth = playerPawn->HealthComponent->GetMaxHealth();
        playerPawn->HealthComponent->currentHealth = maxHealth;
        if (Cheat::Config::Features::GodMode != lastGodModeState) {
            LOG_INFO("God Mode enabled - direct health manipulation");
            lastGodModeState = Cheat::Config::Features::GodMode;
        }
    } else if (!Cheat::Config::Features::GodMode && lastGodModeState) {
        LOG_INFO("God Mode disabled");
        lastGodModeState = Cheat::Config::Features::GodMode;
    }

    // Speed Hack
    if (playerPawn && playerPawn->GetRPawnMovementComponent() && Cheat::Config::Features::OriginalSpeedsSaved) {
        auto moveComp = playerPawn->GetRPawnMovementComponent();
        if (Cheat::Config::Features::SpeedHack) {
            moveComp->MovementSpeedModifier = SDK::FRMutableFloat{ Cheat::Config::Features::SpeedMultiplier, Cheat::Config::Features::SpeedMultiplier, Cheat::Config::Features::SpeedMultiplier };
            if (Cheat::Config::Features::SpeedHack != lastSpeedHackState) {
                LOG_INFO("Speed Hack enabled - %.1fx speed (MovementSpeedModifier: %.1f)", Cheat::Config::Features::SpeedMultiplier, Cheat::Config::Features::originalMovementSpeedModifier);
                lastSpeedHackState = Cheat::Config::Features::SpeedHack;
            }
        } else {
            moveComp->MovementSpeedModifier = Cheat::Config::Features::originalMovementSpeedModifier;
            if (lastSpeedHackState) {
                LOG_INFO("Speed Hack disabled - reset to original speeds (MovementSpeedModifier: %.1f)", Cheat::Config::Features::originalMovementSpeedModifier.CurrentValue);
                lastSpeedHackState = Cheat::Config::Features::SpeedHack;
            }
        }
    }

    // Movement Hacks
    if (playerPawn && playerPawn->GetRPawnMovementComponent() && Cheat::Config::Features::OriginalMovementValuesSaved) {
        auto moveComp = playerPawn->GetRPawnMovementComponent();
        // Slow Immunity
        if (Cheat::Config::Features::SlowImmunity) {
            moveComp->bSlowImmunity = true;
            if (!lastSlowImmunityState) {
                LOG_INFO("Slow Immunity enabled");
                lastSlowImmunityState = true;
            }
        } else {
            moveComp->bSlowImmunity = Cheat::Config::Features::originalSlowImmunity;
            if (lastSlowImmunityState) {
                LOG_INFO("Slow Immunity disabled - restored to original state");
                lastSlowImmunityState = false;
            }
        }
        // Jump Height
        if (Cheat::Config::Features::JumpHeightHack) {
            moveComp->JumpHeight = Cheat::Config::Features::originalJumpHeight * Cheat::Config::Features::JumpHeightMultiplier;
            if (!lastJumpHeightHackState) {
                LOG_INFO("Jump Height Hack enabled - %.1fx jump height", Cheat::Config::Features::JumpHeightMultiplier);
                lastJumpHeightHackState = true;
            }
        } else {
            moveComp->JumpHeight = Cheat::Config::Features::originalJumpHeight;
            if (lastJumpHeightHackState) {
                LOG_INFO("Jump Height Hack disabled - restored to original height");
                lastJumpHeightHackState = false;
            }
        }
        // Dash Speed
        if (Cheat::Config::Features::DashSpeedHack) {
            moveComp->DashSpeed = Cheat::Config::Features::originalDashSpeed * Cheat::Config::Features::DashSpeedMultiplier;
            moveComp->DashTime = Cheat::Config::Features::originalDashTime * Cheat::Config::Features::DashSpeedMultiplier;
            if (!lastDashSpeedHackState) {
                LOG_INFO("Dash Speed Hack enabled - %.1fx dash speed/time", Cheat::Config::Features::DashSpeedMultiplier);
                lastDashSpeedHackState = true;
            }
        } else {
            moveComp->DashSpeed = Cheat::Config::Features::originalDashSpeed;
            moveComp->DashTime = Cheat::Config::Features::originalDashTime;
            if (lastDashSpeedHackState) {
                LOG_INFO("Dash Speed Hack disabled - restored to original dash values");
                lastDashSpeedHackState = false;
            }
        }
    }

    if (shouldDebugLog) {
        LOG_INFO("=== CHEAT STATUS DEBUG ===");
        LOG_INFO("God Mode: %s", Cheat::Config::Features::GodMode ? "ON" : "OFF");
        LOG_INFO("Speed Hack: %s", Cheat::Config::Features::SpeedHack ? "ON" : "OFF");
        LOG_INFO("Aimbot Enabled: %s ", Cheat::Config::Aimbot::enabled ? "ON" : "OFF");
        lastDebugTime = currentTime;
    }
}

} // namespace Services
} // namespace Cheat

