#include "AimbotTab.h"
#include <imgui.h>
#include "../../cheats/Core/Config.h"
#include "../../cheats/Utils/Input.h"
#include <dev/logger.h>

namespace CheatMenu { namespace Tabs {

void AimbotTab() {
    ImGui::Text("Aimbot Configuration");
    ImGui::Separator();

    if (ImGui::Checkbox("Enable Aimbot", &Cheat::Config::Aimbot::enabled)) {
        LOG_INFO("GUI: Aimbot checkbox clicked - new value: %s", Cheat::Config::Aimbot::enabled ? "ENABLED" : "DISABLED");
        LOG_INFO("GUI: Variable address: %p", &Cheat::Config::Aimbot::enabled);
    }
    if (ImGui::Checkbox("Visibility Check", &Cheat::Config::Aimbot::visibilityCheck)) {
        LOG_INFO("GUI: Aimbot Visibility Check %s", Cheat::Config::Aimbot::visibilityCheck ? "ENABLED" : "DISABLED");
    }

    ImGui::Spacing();
    ImGui::Text("Hotkey Configuration");
    ImGui::Separator();

    ImGui::Text("Aimbot Trigger Key:");
    ImGui::SameLine();

    if (Cheat::Config::Hotkeys::IsCapturingHotkey &&
        Cheat::Config::Hotkeys::CurrentHotkeyBeingSet == &Cheat::Config::Hotkeys::AimbotTrigger) {
        ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "Press any key... (ESC to cancel)");
        if (ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows)) {
            BYTE capturedKey = Cheat::Utils::Input::CaptureNextKeyPress();
            if (capturedKey != 0) {
                if (capturedKey == VK_ESCAPE) {
                    Cheat::Config::Hotkeys::IsCapturingHotkey = false;
                    Cheat::Config::Hotkeys::CurrentHotkeyBeingSet = nullptr;
                    LOG_INFO("GUI: Aimbot hotkey capture cancelled");
                } else {
                    Cheat::Config::Hotkeys::AimbotTrigger = capturedKey;
                    Cheat::Config::Hotkeys::IsCapturingHotkey = false;
                    Cheat::Config::Hotkeys::CurrentHotkeyBeingSet = nullptr;
                    LOG_INFO("GUI: Aimbot trigger key set to: %s (VK: %d)",
                        Cheat::Utils::Input::GetKeyName(capturedKey), capturedKey);
                }
            }
        }
    } else {
        ImGui::Text("Current aimbot key: %s", Cheat::Utils::Input::GetKeyName(Cheat::Config::Hotkeys::AimbotTrigger));
        ImGui::SameLine();
        if (ImGui::Button("Set Hotkey")) {
            Cheat::Config::Hotkeys::IsCapturingHotkey = true;
            Cheat::Config::Hotkeys::CurrentHotkeyBeingSet = &Cheat::Config::Hotkeys::AimbotTrigger;
            LOG_INFO("GUI: Started capturing aimbot trigger key");
        }
    }

    ImGui::Spacing();
    ImGui::Text("Targeting Settings");
    ImGui::SliderFloat("Max Distance", &Cheat::Config::Aimbot::maxDistance, 1000.0f, 100000.0f);
}

} } // namespace CheatMenu::Tabs

