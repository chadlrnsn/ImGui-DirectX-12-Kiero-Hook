#include "CheatMenu.h"
#include "../cheats/Core/Config.h"
#include "../cheats/Utils/Input.h"
#include <imgui.h>
#include <dev/logger.h>
#include "Tabs/AimbotTab.h"
#include "Tabs/WeaponsTab.h"
#include "Tabs/GameplayTab.h"
#include "Tabs/OptionsTab.h"
#include "Tabs/MiscTab.h"
#include "Tabs/DebugTab.h"
#include <cheats/Features/WeaponService.h>

namespace CheatMenu {

    // Menu state
    static bool show_menu = false;
    static MenuTab current_tab = MenuTab::AIMBOT;

    // Apply a dark theme for the cheat menu
    void ApplyCheatMenuTheme() {
        ImVec4* colors = ImGui::GetStyle().Colors;
        colors[ImGuiCol_Text] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
        colors[ImGuiCol_TextDisabled] = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
        colors[ImGuiCol_WindowBg] = ImVec4(0.10f, 0.10f, 0.10f, 0.90f);
        colors[ImGuiCol_ChildBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
        colors[ImGuiCol_PopupBg] = ImVec4(0.19f, 0.19f, 0.19f, 0.92f);
        colors[ImGuiCol_Border] = ImVec4(0.19f, 0.19f, 0.19f, 0.29f);
        colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.4f);
        colors[ImGuiCol_FrameBg] = ImVec4(0.05f, 0.05f, 0.05f, 0.54f);
        colors[ImGuiCol_FrameBgHovered] = ImVec4(0.19f, 0.19f, 0.19f, 0.54f);
        colors[ImGuiCol_FrameBgActive] = ImVec4(0.20f, 0.22f, 0.23f, 0.70f);
        colors[ImGuiCol_TitleBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.60f);
        colors[ImGuiCol_TitleBgActive] = ImVec4(0.06f, 0.06f, 0.06f, 0.60f);
        colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.00f, 0.00f, 0.00f, 0.30f);
        colors[ImGuiCol_MenuBarBg] = ImVec4(0.14f, 0.14f, 0.14f, 0.60f);
        colors[ImGuiCol_ScrollbarBg] = ImVec4(0.05f, 0.05f, 0.05f, 0.54f);
        colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.34f, 0.34f, 0.34f, 0.54f);
        colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.40f, 0.40f, 0.40f, 0.54f);
        colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.56f, 0.56f, 0.56f, 0.54f);
        colors[ImGuiCol_CheckMark] = ImVec4(0.33f, 0.67f, 0.86f, 1.00f);
        colors[ImGuiCol_SliderGrab] = ImVec4(0.34f, 0.34f, 0.34f, 0.54f);
        colors[ImGuiCol_SliderGrabActive] = ImVec4(0.56f, 0.56f, 0.56f, 0.54f);
        colors[ImGuiCol_Button] = ImVec4(0.05f, 0.05f, 0.05f, 0.54f);
        colors[ImGuiCol_ButtonHovered] = ImVec4(0.19f, 0.19f, 0.19f, 0.54f);
        colors[ImGuiCol_ButtonActive] = ImVec4(0.20f, 0.22f, 0.23f, 1.00f);
        colors[ImGuiCol_Header] = ImVec4(0.00f, 0.00f, 0.00f, 0.52f);
        colors[ImGuiCol_HeaderHovered] = ImVec4(0.00f, 0.00f, 0.00f, 0.36f);
        colors[ImGuiCol_HeaderActive] = ImVec4(0.20f, 0.22f, 0.23f, 0.33f);
        colors[ImGuiCol_Separator] = ImVec4(0.28f, 0.28f, 0.28f, 0.29f);
        colors[ImGuiCol_SeparatorHovered] = ImVec4(0.44f, 0.44f, 0.44f, 0.29f);
        colors[ImGuiCol_SeparatorActive] = ImVec4(0.40f, 0.44f, 0.47f, 1.00f);
        colors[ImGuiCol_ResizeGrip] = ImVec4(0.28f, 0.28f, 0.28f, 0.29f);
        colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.44f, 0.44f, 0.44f, 0.29f);
        colors[ImGuiCol_ResizeGripActive] = ImVec4(0.40f, 0.44f, 0.47f, 1.00f);
        colors[ImGuiCol_Tab] = ImVec4(0.00f, 0.00f, 0.00f, 0.52f);
        colors[ImGuiCol_TabHovered] = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
        colors[ImGuiCol_TabActive] = ImVec4(0.20f, 0.20f, 0.20f, 0.36f);
        colors[ImGuiCol_TabUnfocused] = ImVec4(0.00f, 0.00f, 0.00f, 0.52f);
        colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
        colors[ImGuiCol_PlotLines] = ImVec4(1.00f, 0.00f, 0.00f, 1.00f);
        colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.00f, 0.00f, 0.00f, 1.00f);
        colors[ImGuiCol_PlotHistogram] = ImVec4(1.00f, 0.00f, 0.00f, 1.00f);
        colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.00f, 0.00f, 0.00f, 1.00f);
        colors[ImGuiCol_TextSelectedBg] = ImVec4(0.20f, 0.22f, 0.23f, 1.00f);
        colors[ImGuiCol_DragDropTarget] = ImVec4(0.33f, 0.67f, 0.86f, 1.00f);
        colors[ImGuiCol_NavHighlight] = ImVec4(1.00f, 0.00f, 0.00f, 1.00f);
        colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.00f, 0.00f, 0.00f, 0.70f);
        colors[ImGuiCol_NavWindowingDimBg] = ImVec4(1.00f, 0.00f, 0.00f, 0.20f);
        colors[ImGuiCol_ModalWindowDimBg] = ImVec4(1.00f, 0.00f, 0.00f, 0.35f);

        ImGuiStyle& style = ImGui::GetStyle();
        style.WindowPadding = ImVec2(15, 5);
        style.IndentSpacing = 25;
        style.GrabMinSize = 10;
        style.ChildBorderSize = 1;
        style.PopupBorderSize = 1;
        style.WindowRounding = 6;
        style.ChildRounding = 4;
        style.FrameRounding = 3;
        style.PopupRounding = 4;
        style.ScrollbarRounding = 9;
        style.ScrollbarSize = 10;
        style.GrabRounding = 3;
        style.WindowBorderSize = 3;
        style.WindowTitleAlign = ImVec2(0.5, 0.5);
    }

    void Toggle() {
        show_menu = !show_menu;
    }

    bool IsVisible() {
        return show_menu;
    }

    void Render() {
        if (!show_menu) return;

        // Initialize GUI scale on first render
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

            // Tab buttons
            if (ImGui::Button("Aimbot")) current_tab = MenuTab::AIMBOT;
            ImGui::SameLine();
            if (ImGui::Button("Weapons")) current_tab = MenuTab::WEAPONS;
            ImGui::SameLine();
            if (ImGui::Button("Gameplay")) current_tab = MenuTab::GAMEPLAY;
            ImGui::SameLine();
            if (ImGui::Button("Options")) current_tab = MenuTab::OPTIONS;
            ImGui::SameLine();
            if (ImGui::Button("Misc")) current_tab = MenuTab::MISC;
            ImGui::SameLine();
            if (ImGui::Button("Debug")) current_tab = MenuTab::DEBUG;

            ImGui::Separator();
            ImGui::Spacing();

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



                  
            
        
        ImGui::End();
    }
    }
} // namespace CheatMenu
