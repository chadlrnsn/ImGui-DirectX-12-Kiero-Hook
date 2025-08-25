#include "OptionsTab.h"
#include <imgui.h>
#include "../../cheats/Core/Config.h"
#include <dev/logger.h>

namespace CheatMenu { namespace Tabs {

void OptionsTab() {
    ImGui::Text("Options & Settings");
    ImGui::Separator();

    ImGui::Text("Interface Settings");
    ImGui::Separator();

    static float previousScale = Cheat::Config::GUI::Scale;
    if (ImGui::SliderFloat("GUI Scale", &Cheat::Config::GUI::Scale, 0.5f, 3.0f, "%.1fx")) {
        ImGuiIO& io = ImGui::GetIO();
        io.FontGlobalScale = Cheat::Config::GUI::Scale;
        ImGui::SetWindowSize(ImVec2(600 * Cheat::Config::GUI::Scale, 400 * Cheat::Config::GUI::Scale));
        LOG_INFO("GUI: GUI Scale changed from %.1fx to %.1fx", previousScale, Cheat::Config::GUI::Scale);
        previousScale = Cheat::Config::GUI::Scale;
    }
    if (ImGui::IsItemHovered()) ImGui::SetTooltip("Adjust the size of the GUI interface\n0.5x = Half size, 1.0x = Normal, 2.0x = Double size");

    ImGui::Spacing();
    ImGui::Text("Current Scale: %.1fx", Cheat::Config::GUI::Scale);

    if (ImGui::Button("Reset to Default (1.0x)")) {
        Cheat::Config::GUI::Scale = 1.0f;
        ImGuiIO& io = ImGui::GetIO();
        io.FontGlobalScale = 1.0f;
        ImGui::SetWindowSize(ImVec2(600 * 1.0f, 400 * 1.0f));
        LOG_INFO("GUI: GUI Scale reset to default (1.0x)");
    }
    if (ImGui::IsItemHovered()) ImGui::SetTooltip("Reset GUI scale to normal size");

    ImGui::SameLine();
    if (ImGui::Button("1.5x")) {
        Cheat::Config::GUI::Scale = 1.5f;
        ImGuiIO& io = ImGui::GetIO();
        io.FontGlobalScale = 1.5f;
        ImGui::SetWindowSize(ImVec2(600 * 1.5f, 400 * 1.5f));
        LOG_INFO("GUI: GUI Scale set to (1.5x)");
    }
}

} } // namespace CheatMenu::Tabs

