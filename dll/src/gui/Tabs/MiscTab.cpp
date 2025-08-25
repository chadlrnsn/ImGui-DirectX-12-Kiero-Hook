#include "MiscTab.h"
#include <imgui.h>
#include "../../cheats/Core/Config.h"
#include "../../cheats/Utils/Input.h"

namespace CheatMenu { namespace Tabs {

void MiscTab() {
    ImGui::Text("Miscellaneous Settings");
    ImGui::Separator();

    ImGui::Text("Hotkeys:");
    ImGui::BulletText("F1: Apply weapon modifications");
    ImGui::BulletText("F2: Toggle aimbot");
    ImGui::BulletText("F3: Dump enemy bones");
    ImGui::BulletText("F4: Show bone database");
    ImGui::BulletText("F5: Log weapon stats (debug)");
    ImGui::BulletText("%s: Hold to activate aimbot", Cheat::Utils::Input::GetKeyName(Cheat::Config::Hotkeys::AimbotTrigger));
    ImGui::BulletText("Insert: Toggle this menu");
    ImGui::BulletText("F9: Exit cheat system (CAUSES GAME CRASH)");
}

} } // namespace CheatMenu::Tabs

