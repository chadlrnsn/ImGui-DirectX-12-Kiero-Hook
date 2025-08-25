#include "CheatMenu.h"
#include "../cheats/Core/Config.h"
#include "../cheats/Utils/Input.h"
#include <imgui.h>
#include <dev/logger.h>
#include <string>

#include "Tabs/AimbotTab.h"
#include "Tabs/WeaponsTab.h"
#include "Tabs/GameplayTab.h"
#include "Tabs/OptionsTab.h"
#include "Tabs/MiscTab.h"
#include "Tabs/DebugTab.h"
#include <cheats/Features/WeaponService.h>
#include "../cheats/Services/GameServices.h"
#include "../dev/imgui/IconsFontAwesome5.h"

static const char* kTabIconAimbot   = ICON_FA_CROSSHAIRS;
static const char* kTabIconWeapons  = ICON_FA_BOMB;
namespace {
    bool g_inputBlockedByMenu = false;
}

static const char* kTabIconGameplay = ICON_FA_WALKING;
static const char* kTabIconOptions  = ICON_FA_COG;
static const char* kTabIconMisc     = ICON_FA_MAGIC;
static const char* kTabIconDebug    = ICON_FA_BUG;

namespace CheatMenu {

    // Menu state
    static bool show_menu = false;
    static MenuTab current_tab = MenuTab::AIMBOT;

    // Apply a dark theme for the cheat menu
    void ApplyCheatMenuTheme() {
        ImGuiStyle& style = ImGui::GetStyle();
        ImVec4* colors = style.Colors;

        // Accent color inspired by common UC menus (electric blue)
        const ImVec4 accent = ImVec4(0.18f, 0.52f, 0.96f, 1.00f);
        const ImVec4 accentHover = ImVec4(0.22f, 0.58f, 1.00f, 1.00f);
        const ImVec4 accentActive = ImVec4(0.10f, 0.42f, 0.88f, 1.00f);

        // Base dark theme
        colors[ImGuiCol_Text]                 = ImVec4(0.94f, 0.94f, 0.94f, 1.00f);
        colors[ImGuiCol_TextDisabled]         = ImVec4(0.60f, 0.60f, 0.60f, 1.00f);
        colors[ImGuiCol_WindowBg]             = ImVec4(0.08f, 0.08f, 0.09f, 0.98f);
        colors[ImGuiCol_ChildBg]              = ImVec4(0.06f, 0.06f, 0.07f, 0.95f);
        colors[ImGuiCol_PopupBg]              = ImVec4(0.08f, 0.08f, 0.09f, 0.98f);
        colors[ImGuiCol_Border]               = ImVec4(0.13f, 0.13f, 0.15f, 1.00f);
        colors[ImGuiCol_BorderShadow]         = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);

        colors[ImGuiCol_FrameBg]              = ImVec4(0.10f, 0.10f, 0.12f, 1.00f);
        colors[ImGuiCol_FrameBgHovered]       = ImVec4(0.14f, 0.14f, 0.17f, 1.00f);
        colors[ImGuiCol_FrameBgActive]        = ImVec4(0.17f, 0.17f, 0.20f, 1.00f);

        colors[ImGuiCol_TitleBg]              = ImVec4(0.06f, 0.06f, 0.07f, 1.00f);
        colors[ImGuiCol_TitleBgActive]        = ImVec4(0.06f, 0.06f, 0.07f, 1.00f);
        colors[ImGuiCol_TitleBgCollapsed]     = ImVec4(0.06f, 0.06f, 0.07f, 1.00f);
        colors[ImGuiCol_MenuBarBg]            = ImVec4(0.08f, 0.08f, 0.09f, 1.00f);

        colors[ImGuiCol_ScrollbarBg]          = ImVec4(0.05f, 0.05f, 0.06f, 1.00f);
        colors[ImGuiCol_ScrollbarGrab]        = ImVec4(0.18f, 0.18f, 0.22f, 1.00f);
        colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.22f, 0.22f, 0.26f, 1.00f);
        colors[ImGuiCol_ScrollbarGrabActive]  = ImVec4(0.26f, 0.26f, 0.30f, 1.00f);

        colors[ImGuiCol_CheckMark]            = accent;
        colors[ImGuiCol_SliderGrab]           = accent;
        colors[ImGuiCol_SliderGrabActive]     = accentActive;

        colors[ImGuiCol_Button]               = ImVec4(0.12f, 0.12f, 0.14f, 1.00f);
        colors[ImGuiCol_ButtonHovered]        = ImVec4(0.16f, 0.16f, 0.19f, 1.00f);
        colors[ImGuiCol_ButtonActive]         = ImVec4(0.12f, 0.22f, 0.36f, 1.00f);

        colors[ImGuiCol_Header]               = ImVec4(0.12f, 0.12f, 0.14f, 1.00f);
        colors[ImGuiCol_HeaderHovered]        = ImVec4(0.16f, 0.16f, 0.19f, 1.00f);
        colors[ImGuiCol_HeaderActive]         = ImVec4(0.12f, 0.22f, 0.36f, 1.00f);

        colors[ImGuiCol_Separator]            = ImVec4(0.20f, 0.20f, 0.23f, 1.00f);
        colors[ImGuiCol_SeparatorHovered]     = accentHover;
        colors[ImGuiCol_SeparatorActive]      = accentActive;

        colors[ImGuiCol_ResizeGrip]           = ImVec4(0.20f, 0.20f, 0.23f, 1.00f);
        colors[ImGuiCol_ResizeGripHovered]    = accentHover;
        colors[ImGuiCol_ResizeGripActive]     = accentActive;

        colors[ImGuiCol_Tab]                  = ImVec4(0.12f, 0.12f, 0.14f, 1.00f);
        colors[ImGuiCol_TabHovered]           = accentHover;
        colors[ImGuiCol_TabActive]            = accent;
        colors[ImGuiCol_TabUnfocused]         = ImVec4(0.12f, 0.12f, 0.14f, 1.00f);
        colors[ImGuiCol_TabUnfocusedActive]   = accentActive;

        colors[ImGuiCol_TextSelectedBg]       = ImVec4(0.20f, 0.22f, 0.30f, 1.00f);
        colors[ImGuiCol_DragDropTarget]       = accent;
        colors[ImGuiCol_NavHighlight]         = accent;
        colors[ImGuiCol_NavWindowingHighlight]= ImVec4(1.00f, 1.00f, 1.00f, 0.10f);
        colors[ImGuiCol_NavWindowingDimBg]    = ImVec4(0.80f, 0.80f, 0.80f, 0.02f);
        colors[ImGuiCol_ModalWindowDimBg]     = ImVec4(0.06f, 0.06f, 0.07f, 0.80f);

        // Layout & rounding tuned for cheat menus
        style.WindowPadding      = ImVec2(18, 12);
        style.FramePadding       = ImVec2(12, 8);
        style.ItemSpacing        = ImVec2(10, 8);
        style.ItemInnerSpacing   = ImVec2(8, 6);
        style.IndentSpacing      = 22;
        style.ScrollbarSize      = 12;

        style.WindowBorderSize   = 1.0f;
        style.ChildBorderSize    = 1.0f;
        style.PopupBorderSize    = 1.0f;
        style.FrameBorderSize    = 0.0f;

        style.WindowRounding     = 8.0f;
        style.ChildRounding      = 8.0f;
        style.FrameRounding      = 6.0f;
        style.PopupRounding      = 6.0f;
        style.ScrollbarRounding  = 12.0f;
        style.GrabRounding       = 6.0f;
        style.TabRounding        = 6.0f;

        style.WindowTitleAlign   = ImVec2(0.5f, 0.5f);
    }

    void Toggle() {
        show_menu = !show_menu;
        auto* pc = Cheat::Services::GameServices::GetPlayerController();
        if (!pc) return;

        // Block/unblock only the game input (not ImGui)
        if (show_menu && !g_inputBlockedByMenu) {
            // Block game input
            pc->ClientIgnoreLookInput(true);
            pc->ClientIgnoreMoveInput(true);
            pc->bShowMouseCursor = true;
            pc->bEnableClickEvents = true;
            pc->bEnableMouseOverEvents = true;
            g_inputBlockedByMenu = true;
        }
        else if (!show_menu && g_inputBlockedByMenu) {
            // Unblock game input
            pc->ClientIgnoreLookInput(false);
            pc->ClientIgnoreMoveInput(false);
            pc->bShowMouseCursor = false;
            pc->bEnableClickEvents = false;
            pc->bEnableMouseOverEvents = false;
            g_inputBlockedByMenu = false;
        }
    }

    bool IsVisible() {
        return show_menu;
    }

    void Render() {
        if (!show_menu) return;

        // Initialize GUI scale on first render
        // Apply theme early so style is active for the whole window
        ApplyCheatMenuTheme();

        static bool scaleInitialized = false;
        if (!scaleInitialized) {
            ImGuiIO& io = ImGui::GetIO();
            io.FontGlobalScale = Cheat::Config::GUI::Scale;
            scaleInitialized = true;
            LOG_INFO("GUI: Initial GUI scale set to %.1fx", Cheat::Config::GUI::Scale);
        }

        // Debug: Log variable addresses and values once

        static bool debugLogged = false;
        if (!debugLogged) {
            LOG_INFO("=== GUI VARIABLE DEBUG ===");
            LOG_INFO("Aimbot::enabled address: %p, value: %s", &Cheat::Config::Aimbot::enabled, Cheat::Config::Aimbot::enabled ? "true" : "false");
            LOG_INFO("Features::GodMode address: %p, value: %s", &Cheat::Config::Features::GodMode, Cheat::Config::Features::GodMode ? "true" : "false");
            debugLogged = true;
        }

        // Apply theme
        ApplyCheatMenuTheme();

        // Main cheat window
        if (ImGui::Begin("Cheat Menu", &show_menu, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoSavedSettings)) {
            // Scale window size based on GUI scale for better usability
            static bool windowSizeSet = false;
            if (!windowSizeSet) {
                float scale = Cheat::Config::GUI::Scale;
                ImGui::SetWindowPos(ImVec2(100, 100), ImGuiCond_Always);
                ImGui::SetWindowSize(ImVec2(600 * scale, 400 * scale), ImGuiCond_Always);
                windowSizeSet = true;
            }

            // Sidebar + content layout
            ImGui::BeginChild("##sidebar", ImVec2(200, 0), true);
            {
                ImGuiStyle& style = ImGui::GetStyle();
                ImVec4 accent = ImVec4(0.18f, 0.52f, 0.96f, 1.0f);
                ImVec4 bgSelected = ImVec4(0.12f, 0.22f, 0.36f, 1.0f);
                ImVec4 bgHover = ImVec4(0.16f, 0.16f, 0.19f, 1.0f);

                auto SidebarButton = [&](MenuTab tab, const char* icon, const char* label){
                    bool selected = (current_tab == tab);
                    ImVec2 pos = ImGui::GetCursorPos();
                    ImVec2 size = ImVec2(ImGui::GetContentRegionAvail().x, 36);

                    // Background for hover/selected
                    ImDrawList* dl = ImGui::GetWindowDrawList();
                    ImVec2 p0 = ImGui::GetCursorScreenPos();
                    ImVec2 p1 = ImVec2(p0.x + size.x, p0.y + size.y);
                    ImU32 col = 0;
                    if (selected) col = ImGui::GetColorU32(bgSelected);
                    else if (ImGui::IsMouseHoveringRect(p0, p1)) col = ImGui::GetColorU32(bgHover);
                    if (col) dl->AddRectFilled(p0, p1, col, style.FrameRounding);

                    // Icon + label
                    ImGui::SetCursorPos(ImVec2(pos.x + 12, pos.y + 8));
                    ImGui::PushStyleColor(ImGuiCol_Text, selected ? accent : ImVec4(0.85f,0.85f,0.90f,1.0f));
                    ImGui::Text("%s  %s", icon, label);
                    ImGui::PopStyleColor();

                    // Invisibility button area for click handling
                    ImGui::SetCursorPos(pos);
                    if (ImGui::InvisibleButton(label, size)) current_tab = tab;

                    ImGui::Spacing();
                };

                SidebarButton(MenuTab::AIMBOT,   kTabIconAimbot,   "Aimbot");
                SidebarButton(MenuTab::WEAPONS,  kTabIconWeapons,  "Weapons");
                SidebarButton(MenuTab::GAMEPLAY, kTabIconGameplay, "Gameplay");
                SidebarButton(MenuTab::OPTIONS,  kTabIconOptions,  "Options");
                SidebarButton(MenuTab::MISC,     kTabIconMisc,     "Misc");
                SidebarButton(MenuTab::DEBUG,    kTabIconDebug,    "Debug");
            }
            ImGui::EndChild();
            ImGui::SameLine();

            ImGui::BeginChild("##content", ImVec2(0, 0), true);

            // Tab content
            switch (current_tab) {
                case MenuTab::AIMBOT: {
                    Tabs::AimbotTab();
                    break;
                }

                case MenuTab::WEAPONS: {
                    Tabs::WeaponsTab();
                    break;
                }

                case MenuTab::GAMEPLAY: {
                    Tabs::GameplayTab();
                    break;
                }

                case MenuTab::OPTIONS: {
                    Tabs::OptionsTab();
                    break;
                }

                case MenuTab::MISC: {
                    Tabs::MiscTab();
                    break;
                }

                case MenuTab::DEBUG: {
                    Tabs::DebugTab();
                    break;
                }

                default: {
                    break;
                }
            }
            ImGui::EndChild();
            ImGui::End();
        }
    }
} // namespace CheatMenu
