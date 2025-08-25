#include "WeaponsTab.h"
#include <imgui.h>
#include "../../cheats/Core/Config.h"
#include "../../cheats/Features/WeaponService.h"

namespace CheatMenu { namespace Tabs {

void WeaponsTab() {
    ImGui::Text("Weapon Modifications");
    ImGui::Separator();

    if (ImGui::Checkbox("Infinite Ammo", &Cheat::Config::Features::InfiniteAmmo)) {
        Cheat::Features::WeaponService::OnWeaponSettingsChanged();
    }
    if (ImGui::IsItemHovered()) ImGui::SetTooltip("No ammo cost when firing");

    if (ImGui::Checkbox("Increased Damage", &Cheat::Config::Features::IncreasedDamage)) {
        Cheat::Features::WeaponService::OnWeaponSettingsChanged();
    }
    if (ImGui::IsItemHovered()) ImGui::SetTooltip("Use slider to apply a damage multiplier");
    if (Cheat::Config::Features::IncreasedDamage) {
        ImGui::Indent();
        if (ImGui::SliderFloat("Damage Multiplier", &Cheat::Config::Features::DamageMultiplier, 0.1f, 10.0f, "x%.2f")) {
            Cheat::Features::WeaponService::OnWeaponSettingsChanged();
        }
        ImGui::Unindent();
    }

    if (ImGui::Checkbox("Crit Multiplier", &Cheat::Config::Features::HighCritMultiplier)) {
        Cheat::Features::WeaponService::OnWeaponSettingsChanged();
    }
    if (ImGui::IsItemHovered()) ImGui::SetTooltip("Use slider to apply a crit multiplier");
    if (Cheat::Config::Features::HighCritMultiplier) {
        ImGui::Indent();
        // Use unique ID scope to avoid label collisions with other sliders named similarly.
        ImGui::PushID("CritMultiplierSlider");
        if (ImGui::SliderFloat("Crit Multiplier Value", &Cheat::Config::Features::CritMultiplier, 0.1f, 10.0f, "x%.2f")) {
            Cheat::Features::WeaponService::OnWeaponSettingsChanged();
        }
        ImGui::PopID();
        ImGui::Unindent();
    }

    if (ImGui::Checkbox("Rate of Fire", &Cheat::Config::Features::RateOfFireOverride)) {
        Cheat::Features::WeaponService::OnWeaponSettingsChanged();
    }
    if (ImGui::IsItemHovered()) ImGui::SetTooltip("Override base rate of fire using the slider below");
    if (Cheat::Config::Features::RateOfFireOverride) {
        ImGui::Indent();
        if (ImGui::SliderFloat("Rate of Fire Value", &Cheat::Config::Features::RateOfFireValue, 0.1f, 100.0f)) {
            Cheat::Features::WeaponService::OnWeaponSettingsChanged();
        }
        ImGui::Unindent();
    }

    if (ImGui::Checkbox("No Cooldown", &Cheat::Config::Features::NoCooldown)) {
        Cheat::Features::WeaponService::OnWeaponSettingsChanged();
    }
    if (ImGui::IsItemHovered()) ImGui::SetTooltip("No weapon cooldown");

    if (ImGui::Checkbox("No Recoil", &Cheat::Config::Features::NoRecoil)) {
        Cheat::Features::WeaponService::OnWeaponSettingsChanged();
    }
    if (ImGui::IsItemHovered()) ImGui::SetTooltip("No recoil + no spread");

    ImGui::Separator();

    if (ImGui::Checkbox("Engine Rifle Heat Management", &Cheat::Config::Features::EngineRifleHeatManagement)) {
        // No immediate call needed; it runs every update when enabled
    }
    if (ImGui::IsItemHovered()) ImGui::SetTooltip("Prevent engine rifle from overheating");
}

} } // namespace CheatMenu::Tabs

