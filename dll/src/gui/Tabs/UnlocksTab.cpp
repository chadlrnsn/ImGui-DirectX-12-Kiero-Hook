#include "UnlocksTab.h"
#include <imgui.h>
#include "../../cheats/Features/UnlockService.h"
#include "../../cheats/Features/SaveService.h"
#include "../../cheats/Features/ResourceService.h"
#include <dev/logger.h>

namespace CheatMenu { namespace Tabs {

void UnlocksTab() {
    ImGui::Text("Unlocks & Progression");
    ImGui::Separator();

    // Unlocks section
    ImGui::Text("Unlocks");
    if (ImGui::Button("Unlock All Abilities")) {
        Cheat::Features::UnlockService::UnlockAllAbilities();
        LOG_INFO("GUI: UnlockAllAbilities() invoked");
    }
    if (ImGui::Button("Unlock All Locked Content")) {
        Cheat::Features::UnlockService::UnlockAllLockedContent();
        LOG_INFO("GUI: UnlockAllLockedContent() invoked");
    }
    if (ImGui::Button("Unlock All Mutators")) {
        Cheat::Features::UnlockService::UnlockAllMutators();
        LOG_INFO("GUI: UnlockAllMutators() invoked");
    }
    if (ImGui::Button("Unlock All Weapon Mods")) {
        Cheat::Features::UnlockService::UnlockAllWeaponMods();
        LOG_INFO("GUI: UnlockAllWeaponMods() invoked");
    }

    ImGui::Separator();

    // Savegame / Soul Fragments
    ImGui::Text("Resources");
    int32_t soulFragments = -1;
    if (Cheat::Features::SaveService::TryGetSoulFragments(soulFragments)) {
        ImGui::Text("Soul Fragments: %d", soulFragments);
        if (ImGui::Button("+1 Soul Fragment")) {
            Cheat::Features::SaveService::IncrementSoulFragments(1);
        }
        static int32_t setSoulValue = 10;
        ImGui::Text("Set to:");
        ImGui::SameLine();
        float avail = ImGui::GetContentRegionAvail().x;
        float width = avail * 0.20f; // responsive: 20% of available width
        if (width < 150.0f) width = 150.0f; // ensure at least room for multiple digits
        if (width > 260.0f) width = 260.0f; // avoid overly wide fields
        ImGui::SetNextItemWidth(width);
        if (ImGui::InputInt("##SetSoul", &setSoulValue)) {
            if (setSoulValue < 0) setSoulValue = 0;
        }
        ImGui::SameLine();
        if (ImGui::Button("Apply")) {
            Cheat::Features::SaveService::SetSoulFragments(setSoulValue);
        }
    } else {
        ImGui::Text("Soul Fragments: <unavailable>");
    }

    // In-run resources (temporary): Gold and Keys
    static int32_t goldToAdd = 1000;
    static int32_t keysToAdd = 5;

    // Gold input + apply
    ImGui::Text("Add Gold:");
    ImGui::SameLine();
    {
        float avail = ImGui::GetContentRegionAvail().x;
        float width = avail * 0.20f;
        if (width < 150.0f) width = 150.0f;
        if (width > 260.0f) width = 260.0f;
        ImGui::SetNextItemWidth(width);
        if (ImGui::InputInt("##GoldToAdd", &goldToAdd)) {
            if (goldToAdd < 0) goldToAdd = 0;
        }
        ImGui::SameLine();
        if (ImGui::Button("Apply##Gold")) {
            Cheat::Features::ResourceService::AddGold(goldToAdd);
            LOG_INFO("GUI: AddGold(%d) invoked", goldToAdd);
        }
    }

    // Keys input + apply
    ImGui::Text("Add Keys:");
    ImGui::SameLine();
    {
        float avail = ImGui::GetContentRegionAvail().x;
        float width = avail * 0.20f;
        if (width < 150.0f) width = 150.0f;
        if (width > 260.0f) width = 260.0f;
        ImGui::SetNextItemWidth(width);
        if (ImGui::InputInt("##KeysToAdd", &keysToAdd)) {
            if (keysToAdd < 0) keysToAdd = 0;
        }
        ImGui::SameLine();
        if (ImGui::Button("Apply##Keys")) {
            Cheat::Features::ResourceService::AddKeys(keysToAdd);
            LOG_INFO("GUI: AddKeys(%d) invoked", keysToAdd);
        }
    }
}

} } // namespace CheatMenu::Tabs

