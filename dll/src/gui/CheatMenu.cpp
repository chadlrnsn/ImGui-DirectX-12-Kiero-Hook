#include "CheatMenu.h"
#include "../cheats/Core/Config.h"
#include <imgui.h>
#include <dev/logger.h>

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
            ImGui::SetWindowPos(ImVec2(100, 100), ImGuiCond_Once);
            ImGui::SetWindowSize(ImVec2(600, 400), ImGuiCond_Once);

            // Tab buttons
            if (ImGui::Button("Aimbot")) current_tab = MenuTab::AIMBOT;
            ImGui::SameLine();
            if (ImGui::Button("Features")) current_tab = MenuTab::FEATURES;
            ImGui::SameLine();
            if (ImGui::Button("Misc")) current_tab = MenuTab::MISC;
            ImGui::SameLine();
            if (ImGui::Button("Debug")) current_tab = MenuTab::DEBUG;

            ImGui::Separator();
            ImGui::Spacing();

            // Tab content
            switch (current_tab) {
                case MenuTab::AIMBOT: {
                    ImGui::Text("Aimbot Configuration");
                    ImGui::Separator();

                    // Debug: Show current value before checkbox
                    ImGui::Text("Current aimbot value: %s (addr: %p)", Cheat::Config::Aimbot::enabled ? "TRUE" : "FALSE", &Cheat::Config::Aimbot::enabled);

                    if (ImGui::Checkbox("Enable Aimbot", &Cheat::Config::Aimbot::enabled)) {
                        LOG_INFO("GUI: Aimbot checkbox clicked - new value: %s", Cheat::Config::Aimbot::enabled ? "ENABLED" : "DISABLED");
                        LOG_INFO("GUI: Variable address: %p", &Cheat::Config::Aimbot::enabled);
                    }
                    if (ImGui::Checkbox("Smooth Aiming", &Cheat::Config::Aimbot::smoothEnabled)) {
                        LOG_INFO("GUI: Aimbot Smooth Aiming %s", Cheat::Config::Aimbot::smoothEnabled ? "ENABLED" : "DISABLED");
                    }
                    if (ImGui::Checkbox("Visibility Check", &Cheat::Config::Aimbot::visibilityCheck)) {
                        LOG_INFO("GUI: Aimbot Visibility Check %s", Cheat::Config::Aimbot::visibilityCheck ? "ENABLED" : "DISABLED");
                    }
                    if (ImGui::Checkbox("Draw FOV Circle", &Cheat::Config::Aimbot::drawFOV)) {
                        LOG_INFO("GUI: Aimbot FOV Circle %s", Cheat::Config::Aimbot::drawFOV ? "ENABLED" : "DISABLED");
                    }
                    
                    ImGui::Spacing();
                    ImGui::Text("Targeting Settings");
                    ImGui::SliderFloat("Max Distance", &Cheat::Config::Aimbot::maxDistance, 1000.0f, 100000.0f);
                    ImGui::SliderFloat("FOV Radius", &Cheat::Config::Aimbot::fovRadius, 1.0f, 180.0f);
                    ImGui::SliderFloat("Smooth Factor", &Cheat::Config::Aimbot::smoothFactor, 1.0f, 20.0f);
                    ImGui::SliderFloat("Max Turn Speed", &Cheat::Config::Aimbot::maxTurnSpeed, 100.0f, 10000.0f);
                    
                    ImGui::Spacing();
                    ImGui::Text("Aim Zones");
                    ImGui::Checkbox("Head", &Cheat::Config::Aimbot::aimZones.head);
                    ImGui::SameLine();
                    ImGui::Checkbox("Chest", &Cheat::Config::Aimbot::aimZones.chest);
                    ImGui::SameLine();
                    ImGui::Checkbox("Body", &Cheat::Config::Aimbot::aimZones.body);
                    
                    break;
                }
                
                case MenuTab::FEATURES: {
                    ImGui::Text("Game Features");
                    ImGui::Separator();

                    if (ImGui::Checkbox("God Mode", &Cheat::Config::Features::GodMode)) {
                        LOG_INFO("GUI: God Mode %s", Cheat::Config::Features::GodMode ? "ENABLED" : "DISABLED");
                    }
                    if (ImGui::IsItemHovered()) {
                        ImGui::SetTooltip("Enable invincibility using CheatManager->God()");
                    }

                    if (ImGui::Checkbox("Speed Hack", &Cheat::Config::Features::SpeedHack)) {
                        LOG_INFO("GUI: Speed Hack %s", Cheat::Config::Features::SpeedHack ? "ENABLED" : "DISABLED");
                    }
                    if (ImGui::IsItemHovered()) {
                        ImGui::SetTooltip("Multiply movement speed by configurable amount");
                    }

                    // Speed multiplier slider (only show when speed hack is enabled)
                    if (Cheat::Config::Features::SpeedHack) {
                        ImGui::Indent();
                        if (ImGui::SliderFloat("Speed Multiplier", &Cheat::Config::Features::SpeedMultiplier, 0.1f, 10.0f, "%.1fx")) {
                            LOG_INFO("GUI: Speed Multiplier changed to %.1fx", Cheat::Config::Features::SpeedMultiplier);
                        }
                        if (ImGui::IsItemHovered()) {
                            ImGui::SetTooltip("1.0x = normal speed, 2.0x = double speed, etc.");
                        }

                        // Show original speeds if captured
                        if (Cheat::Config::Features::OriginalSpeedsSaved) {
                            ImGui::Text("Original speeds: Walk=%.0f, Accel=%.0f",
                                Cheat::Config::Features::OriginalMaxWalkSpeed,
                                Cheat::Config::Features::OriginalMaxAcceleration);
                        } else {
                            ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "Original speeds not captured yet");
                        }
                        ImGui::Unindent();
                    }

                    // Weapon Modifications Section
                    ImGui::Separator();
                    ImGui::Text("Weapon Modifications");
                    ImGui::Separator();

                    if (ImGui::Checkbox("Infinite Ammo", &Cheat::Config::Features::InfiniteAmmo)) {
                        LOG_INFO("GUI: Infinite Ammo %s", Cheat::Config::Features::InfiniteAmmo ? "ENABLED" : "DISABLED");
                    }
                    if (ImGui::IsItemHovered()) {
                        ImGui::SetTooltip("No ammo cost when firing");
                    }

                    if (ImGui::Checkbox("Increased Damage", &Cheat::Config::Features::IncreasedDamage)) {
                        LOG_INFO("GUI: Increased Damage %s", Cheat::Config::Features::IncreasedDamage ? "ENABLED" : "DISABLED");
                    }
                    if (ImGui::IsItemHovered()) {
                        ImGui::SetTooltip("Massively increased weapon damage");
                    }

                    if (ImGui::Checkbox("High Critical Multiplier", &Cheat::Config::Features::HighCritMultiplier)) {
                        LOG_INFO("GUI: High Critical Multiplier %s", Cheat::Config::Features::HighCritMultiplier ? "ENABLED" : "DISABLED");
                    }
                    if (ImGui::IsItemHovered()) {
                        ImGui::SetTooltip("Extremely high critical hit multiplier");
                    }

                    if (ImGui::Checkbox("Fast Rate of Fire", &Cheat::Config::Features::FastRateOfFire)) {
                        LOG_INFO("GUI: Fast Rate of Fire %s", Cheat::Config::Features::FastRateOfFire ? "ENABLED" : "DISABLED");
                    }
                    if (ImGui::IsItemHovered()) {
                        ImGui::SetTooltip("Max rate of fire");
                    }

                    if (ImGui::Checkbox("No Cooldown", &Cheat::Config::Features::NoCooldown)) {
                        LOG_INFO("GUI: No Cooldown %s", Cheat::Config::Features::NoCooldown ? "ENABLED" : "DISABLED");
                    }
                    if (ImGui::IsItemHovered()) {
                        ImGui::SetTooltip("No weapon cooldown");
                    }

                    if (ImGui::Checkbox("No Recoil", &Cheat::Config::Features::NoRecoil)) {
                        LOG_INFO("GUI: No Recoil %s", Cheat::Config::Features::NoRecoil ? "ENABLED" : "DISABLED");
                    }
                    if (ImGui::IsItemHovered()) {
                        ImGui::SetTooltip("No recoil + instant recovery + perfect accuracy");
                    }

                    if (ImGui::Checkbox("Instant Reload", &Cheat::Config::Features::InstantReload)) {
                        LOG_INFO("GUI: Instant Reload %s", Cheat::Config::Features::InstantReload ? "ENABLED" : "DISABLED");
                    }
                    if (ImGui::IsItemHovered()) {
                        ImGui::SetTooltip("Instant weapon reload");
                    }

                    ImGui::Separator();

                    if (ImGui::Checkbox("Engine Rifle Heat Management", &Cheat::Config::Features::EngineRifleHeatManagement)) {
                        LOG_INFO("GUI: Engine Rifle Heat Management %s", Cheat::Config::Features::EngineRifleHeatManagement ? "ENABLED" : "DISABLED");
                    }
                    if (ImGui::IsItemHovered()) {
                        ImGui::SetTooltip("Prevent engine rifle from overheating");
                    }
                    
                    break;
                }
                
                case MenuTab::MISC: {
                    ImGui::Text("Miscellaneous Settings");
                    ImGui::Separator();
                    
                    ImGui::Text("Hotkeys:");
                    ImGui::BulletText("F1: Apply weapon modifications");
                    ImGui::BulletText("F2: Toggle aimbot");
                    ImGui::BulletText("F3: Dump enemy bones");
                    ImGui::BulletText("F4: Show bone database");
                    ImGui::BulletText("Mouse4: Hold to activate aimbot");
                    ImGui::BulletText("Insert: Toggle this menu");
                    ImGui::BulletText("F9: Exit cheat system");
                    
                    break;
                }
                
                case MenuTab::DEBUG: {
                    ImGui::Text("Debug Information");
                    ImGui::Separator();

                    ImGui::Checkbox("Enable Math Logging", &Cheat::Config::Debug::enableMathLogging);

                    ImGui::Spacing();
                    ImGui::Text("Manual Toggle Tests:");
                    if (ImGui::Button("Toggle Aimbot (Test)")) {
                        Cheat::Config::Aimbot::enabled = !Cheat::Config::Aimbot::enabled;
                        LOG_INFO("DEBUG: Manual aimbot toggle - now %s", Cheat::Config::Aimbot::enabled ? "ENABLED" : "DISABLED");
                    }
                    if (ImGui::Button("Toggle God Mode (Test)")) {
                        Cheat::Config::Features::GodMode = !Cheat::Config::Features::GodMode;
                        LOG_INFO("DEBUG: Manual god mode toggle - now %s", Cheat::Config::Features::GodMode ? "ENABLED" : "DISABLED");
                    }

                    ImGui::Spacing();
                    ImGui::Text("Cheat Status (Real Values):");
                    ImGui::Text("God Mode: %s", Cheat::Config::Features::GodMode ? "ENABLED" : "Disabled");
                    ImGui::Text("Speed Hack: %s (%.1fx)",
                        Cheat::Config::Features::SpeedHack ? "ENABLED" : "Disabled",
                        Cheat::Config::Features::SpeedMultiplier);
                    ImGui::Text("Weapon Modifications:");
                    ImGui::Indent();
                    ImGui::Text("- Infinite Ammo: %s", Cheat::Config::Features::InfiniteAmmo ? "ON" : "OFF");
                    ImGui::Text("- Increased Damage: %s", Cheat::Config::Features::IncreasedDamage ? "ON" : "OFF");
                    ImGui::Text("- High Crit Multiplier: %s", Cheat::Config::Features::HighCritMultiplier ? "ON" : "OFF");
                    ImGui::Text("- Fast Rate of Fire: %s", Cheat::Config::Features::FastRateOfFire ? "ON" : "OFF");
                    ImGui::Text("- No Cooldown: %s", Cheat::Config::Features::NoCooldown ? "ON" : "OFF");
                    ImGui::Text("- No Recoil (+ Recovery + Accuracy): %s", Cheat::Config::Features::NoRecoil ? "ON" : "OFF");
                    ImGui::Text("- Instant Reload: %s", Cheat::Config::Features::InstantReload ? "ON" : "OFF");
                    ImGui::Unindent();
                    ImGui::Text("Engine Rifle Heat Mgmt: %s", Cheat::Config::Features::EngineRifleHeatManagement ? "ENABLED" : "Disabled");
                    ImGui::Text("Aimbot: %s", Cheat::Config::Aimbot::enabled ? "ENABLED" : "Disabled");

                    if (Cheat::Config::Features::OriginalSpeedsSaved) {
                        ImGui::Text("Original Speeds: Walk=%.0f, Accel=%.0f",
                            Cheat::Config::Features::OriginalMaxWalkSpeed,
                            Cheat::Config::Features::OriginalMaxAcceleration);
                    }

                    ImGui::Spacing();
                    ImGui::Text("Game State:");
                    
                    char engineBuf[256];
                    sprintf_s(engineBuf, sizeof(engineBuf), "Engine: %p", Cheat::Config::GameState::g_pEngine);
                    ImGui::Text(engineBuf);
                    
                    char worldBuf[256];
                    sprintf_s(worldBuf, sizeof(worldBuf), "World: %p", Cheat::Config::GameState::g_pWorld);
                    ImGui::Text(worldBuf);
                    
                    char controllerBuf[256];
                    sprintf_s(controllerBuf, sizeof(controllerBuf), "Controller: %p", Cheat::Config::GameState::g_pMyController);
                    ImGui::Text(controllerBuf);
                    
                    char pawnBuf[256];
                    sprintf_s(pawnBuf, sizeof(pawnBuf), "Pawn: %p", Cheat::Config::GameState::g_pMyPawn);
                    ImGui::Text(pawnBuf);
                    
                    char targetsBuf[256];
                    sprintf_s(targetsBuf, sizeof(targetsBuf), "Targets: %zu", Cheat::Config::GameState::g_TargetsList.size());
                    ImGui::Text(targetsBuf);
                    
                    char currentTargetBuf[256];
                    sprintf_s(currentTargetBuf, sizeof(currentTargetBuf), "Current Target: %p", Cheat::Config::GameState::g_pCurrentTarget);
                    ImGui::Text(currentTargetBuf);
                    
                    break;
                }
            }
        }
        ImGui::End();
    }
    
} // namespace CheatMenu
