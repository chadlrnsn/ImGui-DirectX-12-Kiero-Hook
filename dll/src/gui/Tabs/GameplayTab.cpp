#include "GameplayTab.h"
#include <imgui.h>
#include "../../cheats/Core/Config.h"
#include <dev/logger.h>

namespace CheatMenu { namespace Tabs {

void GameplayTab() {
    ImGui::Text("Gameplay Modifications");
    ImGui::Separator();

    if (ImGui::Checkbox("God Mode", &Cheat::Config::Features::GodMode)) {
        LOG_INFO("GUI: God Mode %s", Cheat::Config::Features::GodMode ? "ENABLED" : "DISABLED");
    }
    if (ImGui::IsItemHovered()) ImGui::SetTooltip("Always max health)");

    ImGui::Separator();
    ImGui::Text("Movement Hacks");
    ImGui::Separator();

    if (ImGui::Checkbox("Speed Hack", &Cheat::Config::Features::SpeedHack)) {
        LOG_INFO("GUI: Speed Hack %s", Cheat::Config::Features::SpeedHack ? "ENABLED" : "DISABLED");
    }
    if (ImGui::IsItemHovered()) ImGui::SetTooltip("Multiply movement speed by configurable amount");

    if (Cheat::Config::Features::SpeedHack) {
        ImGui::Indent();
        if (ImGui::SliderFloat("Speed Multiplier", &Cheat::Config::Features::SpeedMultiplier, 0.1f, 10.0f, "%.1fx")) {
            LOG_INFO("GUI: Speed Multiplier changed to %.1fx", Cheat::Config::Features::SpeedMultiplier);
        }
        if (ImGui::IsItemHovered()) ImGui::SetTooltip("1.0x = normal speed, 2.0x = double speed, etc.");

        if (Cheat::Config::Features::OriginalSpeedsSaved) {
            ImGui::Text("Original speeds: MovementSpeedModifier=%.1f", Cheat::Config::Features::originalMovementSpeedModifier);
        } else {
            ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "Original speeds not captured yet");
        }
        ImGui::Unindent();
    }

    if (ImGui::Checkbox("Slow Immunity", &Cheat::Config::Features::SlowImmunity)) {
        LOG_INFO("GUI: Slow Immunity %s", Cheat::Config::Features::SlowImmunity ? "ENABLED" : "DISABLED");
    }
    if (ImGui::IsItemHovered()) ImGui::SetTooltip("Immunity to slow effects");

    if (ImGui::Checkbox("Jump Height Hack", &Cheat::Config::Features::JumpHeightHack)) {
        LOG_INFO("GUI: Jump Height Hack %s", Cheat::Config::Features::JumpHeightHack ? "ENABLED" : "DISABLED");
    }
    if (ImGui::IsItemHovered()) ImGui::SetTooltip("Multiply jump height");

    if (Cheat::Config::Features::JumpHeightHack) {
        ImGui::Indent();
        if (ImGui::SliderFloat("Jump Height Multiplier", &Cheat::Config::Features::JumpHeightMultiplier, 0.1f, 10.0f, "%.1fx")) {
            LOG_INFO("GUI: Jump Height Multiplier changed to %.1fx", Cheat::Config::Features::JumpHeightMultiplier);
        }
        if (ImGui::IsItemHovered()) ImGui::SetTooltip("1.0x = normal jump, 2.0x = double jump height, etc.");
        ImGui::Unindent();
    }

    if (ImGui::Checkbox("Dash Distance", &Cheat::Config::Features::DashSpeedHack)) {
        LOG_INFO("GUI: Dash Speed Hack %s", Cheat::Config::Features::DashSpeedHack ? "ENABLED" : "DISABLED");
    }
    if (ImGui::IsItemHovered()) ImGui::SetTooltip("Multiply dash distance");

    if (Cheat::Config::Features::DashSpeedHack) {
        ImGui::Indent();
        if (ImGui::SliderFloat("Dash Distance Multiplier", &Cheat::Config::Features::DashSpeedMultiplier, 0.1f, 10.0f, "%.1fx")) {
            LOG_INFO("GUI: Dash Speed Multiplier changed to %.1fx", Cheat::Config::Features::DashSpeedMultiplier);
        }
        if (ImGui::IsItemHovered()) ImGui::SetTooltip("1.0x = normal dash, 2.0x = double dash speed/time, etc.");
        ImGui::Unindent();
    }
}

} } // namespace CheatMenu::Tabs

