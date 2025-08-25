#include "DebugTab.h"
#include <imgui.h>
#include "../../cheats/Core/Config.h"
#include "../../cheats/Services/GameServices.h"
#include "../../cheats/Features/SaveService.h"


namespace CheatMenu { namespace Tabs {

void DebugTab() {
    ImGui::Text("Debug Information");
    ImGui::Separator();

    ImGui::Checkbox("Enable Math Logging", &Cheat::Config::Debug::enableMathLogging);

    ImGui::Spacing();
    ImGui::Text("Manual Toggle Tests:");

    ImGui::Spacing();
    ImGui::Text("Cheat Status (Real Values):");
    ImGui::Text("God Mode: %s", Cheat::Config::Features::GodMode ? "ENABLED" : "Disabled");
    ImGui::Text("Speed Hack: %s (%.1fx)",
        Cheat::Config::Features::SpeedHack ? "ENABLED" : "Disabled",
        Cheat::Config::Features::SpeedMultiplier);
    ImGui::Text("Movement Hacks:");
    ImGui::Indent();
    ImGui::Text("- Slow Immunity: %s", Cheat::Config::Features::SlowImmunity ? "ON" : "OFF");
    ImGui::Text("- Jump Height Hack: %s (%.1fx)",
        Cheat::Config::Features::JumpHeightHack ? "ON" : "OFF",
        Cheat::Config::Features::JumpHeightMultiplier);
    ImGui::Text("- Dash Speed Hack: %s (%.1fx)",
        Cheat::Config::Features::DashSpeedHack ? "ON" : "OFF",
        Cheat::Config::Features::DashSpeedMultiplier);
    ImGui::Unindent();
    ImGui::Text("Weapon Modifications:");
    ImGui::Indent();
    ImGui::Text("- Infinite Ammo: %s", Cheat::Config::Features::InfiniteAmmo ? "ON" : "OFF");
    ImGui::Text("- Increased Damage: %s", Cheat::Config::Features::IncreasedDamage ? "ON" : "OFF");
    ImGui::Text("- High Crit Multiplier: %s", Cheat::Config::Features::HighCritMultiplier ? "ON" : "OFF");
    ImGui::Text("- Rate of Fire Override: %s", Cheat::Config::Features::RateOfFireOverride ? "ON" : "OFF");
    ImGui::Text("- No Cooldown: %s", Cheat::Config::Features::NoCooldown ? "ON" : "OFF");
    ImGui::Text("- No Recoil (+ Recovery + Accuracy): %s", Cheat::Config::Features::NoRecoil ? "ON" : "OFF");
    ImGui::Text("- Instant Reload: %s", Cheat::Config::Features::InstantReload ? "ON" : "OFF");
    ImGui::Unindent();
    ImGui::Text("Engine Rifle Heat Mgmt: %s", Cheat::Config::Features::EngineRifleHeatManagement ? "ENABLED" : "Disabled");
    ImGui::Text("Aimbot: %s", Cheat::Config::Aimbot::enabled ? "ENABLED" : "Disabled");

    if (Cheat::Config::Features::OriginalSpeedsSaved) {
        ImGui::Text("Original Speeds: MovementSpeedModifier=%.1f", Cheat::Config::Features::originalMovementSpeedModifier);
    }
    if (Cheat::Config::Features::OriginalMovementValuesSaved) {
        ImGui::Text("Original Movement: Jump=%.1f, Dash=%.1f/%.1f, SlowImmunity=%s",
            Cheat::Config::Features::originalJumpHeight,
            Cheat::Config::Features::originalDashSpeed,
            Cheat::Config::Features::originalDashTime,
            Cheat::Config::Features::originalSlowImmunity ? "true" : "false");
    }

    ImGui::Spacing();
    ImGui::Text("Game State:");
    ImGui::Text("Engine: %p", Cheat::Services::GameServices::GetEngine());
    ImGui::Text("World: %p", Cheat::Services::GameServices::GetWorld());
    ImGui::Text("Controller: %p", Cheat::Services::GameServices::GetPlayerController());
    ImGui::Text("Pawn: %p", Cheat::Services::GameServices::GetPlayerPawn());
    ImGui::Text("Targets: %zu", Cheat::Config::GameState::g_TargetsList.size());
    ImGui::Text("Current Target: %p", Cheat::Config::GameState::g_pCurrentTarget);

    int32_t soulFragments = -1;
    if (Cheat::Features::SaveService::TryGetSoulFragments(soulFragments)) {
        ImGui::Text("Soul Fragments: %d", soulFragments);
        if (ImGui::Button("+1 Soul Fragment")) {
            Cheat::Features::SaveService::IncrementSoulFragments(1);
        }
    } else {
        ImGui::Text("Soul Fragments: <unavailable>");
    }
}

} } // namespace CheatMenu::Tabs

