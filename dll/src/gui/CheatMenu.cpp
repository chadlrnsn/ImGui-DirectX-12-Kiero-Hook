#include "CheatMenu.h"
#include "../cheats/Core/Config.h"
#include "../cheats/Utils/Input.h"
#include <imgui.h>
#include <dev/logger.h>
#include "../cheats/Features/WeaponManager.h"

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
                    ImGui::Text("Aimbot Configuration");
                    ImGui::Separator();

                    if (ImGui::Checkbox("Enable Aimbot", &Cheat::Config::Aimbot::enabled)) {
                        LOG_INFO("GUI: Aimbot checkbox clicked - new value: %s", Cheat::Config::Aimbot::enabled ? "ENABLED" : "DISABLED");
                        LOG_INFO("GUI: Variable address: %p", &Cheat::Config::Aimbot::enabled);
                    }
                    if (ImGui::Checkbox("Visibility Check", &Cheat::Config::Aimbot::visibilityCheck)) {
                        LOG_INFO("GUI: Aimbot Visibility Check %s", Cheat::Config::Aimbot::visibilityCheck ? "ENABLED" : "DISABLED");
                    }
                    // if (ImGui::Checkbox("Draw FOV Circle", &Cheat::Config::Aimbot::drawFOV)) {
                    //     LOG_INFO("GUI: Aimbot FOV Circle %s", Cheat::Config::Aimbot::drawFOV ? "ENABLED" : "DISABLED");
                    // }

                    ImGui::Spacing();
                    ImGui::Text("Hotkey Configuration");
                    ImGui::Separator();

                    // Aimbot trigger key configuration
                    ImGui::Text("Aimbot Trigger Key:");
                    ImGui::SameLine();

                    if (Cheat::Config::Hotkeys::IsCapturingHotkey &&
                        Cheat::Config::Hotkeys::CurrentHotkeyBeingSet == &Cheat::Config::Hotkeys::AimbotTrigger) {
                        // Currently capturing input
                        ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "Press any key... (ESC to cancel)");

                        // Check for key input (only when menu is focused)
                        if (ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows)) {
                            BYTE capturedKey = Cheat::Utils::Input::CaptureNextKeyPress();
                            if (capturedKey != 0) {
                                if (capturedKey == VK_ESCAPE) {
                                    // Cancel key capture
                                    Cheat::Config::Hotkeys::IsCapturingHotkey = false;
                                    Cheat::Config::Hotkeys::CurrentHotkeyBeingSet = nullptr;
                                    LOG_INFO("GUI: Aimbot hotkey capture cancelled");
                                } else {
                                    // Set new key
                                    Cheat::Config::Hotkeys::AimbotTrigger = capturedKey;
                                    Cheat::Config::Hotkeys::IsCapturingHotkey = false;
                                    Cheat::Config::Hotkeys::CurrentHotkeyBeingSet = nullptr;
                                    LOG_INFO("GUI: Aimbot trigger key set to: %s (VK: %d)",
                                        Cheat::Utils::Input::GetKeyName(capturedKey), capturedKey);
                                }
                            }
                        }
                    } else {
                        // Show current key and set button
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
                    //ImGui::SliderFloat("FOV Radius (kinda buggy, will fix soon)", &Cheat::Config::Aimbot::fovRadius, 1.0f, 180.0f);
                    // ImGui::SliderFloat("Turn Speed (Lower = Smoother, Higher = Fast Snapping)", &Cheat::Config::Aimbot::maxTurnSpeed, 100.0f, 10000.0f);

                    // ImGui::Spacing();
                    // ImGui::Text("Aim Zones");
                    // ImGui::Checkbox("Head", &Cheat::Config::Aimbot::aimZones.head);
                    // ImGui::SameLine();
                    // ImGui::Checkbox("Chest", &Cheat::Config::Aimbot::aimZones.chest);
                    // ImGui::SameLine();
                    // ImGui::Checkbox("Body", &Cheat::Config::Aimbot::aimZones.body);

                    break;
                }

                case MenuTab::WEAPONS: {
                    ImGui::Text("Weapon Modifications");
                    ImGui::Separator();

                    if (ImGui::Checkbox("Infinite Ammo", &Cheat::Config::Features::InfiniteAmmo)) {
                        LOG_INFO("GUI: Infinite Ammo %s", Cheat::Config::Features::InfiniteAmmo ? "ENABLED" : "DISABLED");
                        Cheat::Features::WeaponManager::OnWeaponSettingsChanged();
                    }
                    if (ImGui::IsItemHovered()) ImGui::SetTooltip("No ammo cost when firing");

                    if (ImGui::Checkbox("Increased Damage", &Cheat::Config::Features::IncreasedDamage)) {
                        LOG_INFO("GUI: Increased Damage %s", Cheat::Config::Features::IncreasedDamage ? "ENABLED" : "DISABLED");
                        Cheat::Features::WeaponManager::OnWeaponSettingsChanged();
                    }
                    if (ImGui::IsItemHovered()) ImGui::SetTooltip("Use slider to apply a damage multiplier");
                    if (Cheat::Config::Features::IncreasedDamage) {
                        ImGui::Indent();
                        if (ImGui::SliderFloat("Damage Multiplier", &Cheat::Config::Features::DamageMultiplier, 0.1f, 10.0f, "x%.2f")) {
                            Cheat::Features::WeaponManager::OnWeaponSettingsChanged();
                        }
                        ImGui::Unindent();
                    }

                    if (ImGui::Checkbox("Crit Multiplier", &Cheat::Config::Features::HighCritMultiplier)) {
                        LOG_INFO("GUI: Crit Multiplier %s", Cheat::Config::Features::HighCritMultiplier ? "ENABLED" : "DISABLED");
                        Cheat::Features::WeaponManager::OnWeaponSettingsChanged();
                    }
                    if (ImGui::IsItemHovered()) ImGui::SetTooltip("Use slider to apply a crit multiplier");
                    if (Cheat::Config::Features::HighCritMultiplier) {
                        ImGui::Indent();
                        if (ImGui::SliderFloat("Crit Multiplier", &Cheat::Config::Features::CritMultiplier, 0.1f, 10.0f, "x%.2f")) {
                            Cheat::Features::WeaponManager::OnWeaponSettingsChanged();
                        }
                        ImGui::Unindent();
                    }

                    if (ImGui::Checkbox("Rate of Fire", &Cheat::Config::Features::RateOfFireOverride)) {
                        Cheat::Features::WeaponManager::OnWeaponSettingsChanged();
                    }
                    if (ImGui::IsItemHovered()) ImGui::SetTooltip("Override base rate of fire using the slider below");
                    if (Cheat::Config::Features::RateOfFireOverride) {
                        ImGui::Indent();
                        if (ImGui::SliderFloat("Rate of Fire Value", &Cheat::Config::Features::RateOfFireValue, 0.1f, 100.0f)) {
                            Cheat::Features::WeaponManager::OnWeaponSettingsChanged();
                        }
                        ImGui::Unindent();
                    }

                    if (ImGui::Checkbox("No Cooldown", &Cheat::Config::Features::NoCooldown)) {
                        LOG_INFO("GUI: No Cooldown %s", Cheat::Config::Features::NoCooldown ? "ENABLED" : "DISABLED");
                        Cheat::Features::WeaponManager::OnWeaponSettingsChanged();
                    }
                    if (ImGui::IsItemHovered()) ImGui::SetTooltip("No weapon cooldown");

                    if (ImGui::Checkbox("No Recoil", &Cheat::Config::Features::NoRecoil)) {
                        LOG_INFO("GUI: No Recoil %s", Cheat::Config::Features::NoRecoil ? "ENABLED" : "DISABLED");
                        Cheat::Features::WeaponManager::OnWeaponSettingsChanged();
                    }
                    if (ImGui::IsItemHovered()) ImGui::SetTooltip("No recoil + no spread");

                    ImGui::Separator();

                    if (ImGui::Checkbox("Engine Rifle Heat Management", &Cheat::Config::Features::EngineRifleHeatManagement)) {
                        LOG_INFO("GUI: Engine Rifle Heat Management %s", Cheat::Config::Features::EngineRifleHeatManagement ? "ENABLED" : "DISABLED");
                    }
                    if (ImGui::IsItemHovered()) ImGui::SetTooltip("Prevent engine rifle from overheating");

                    break;
                }

                case MenuTab::GAMEPLAY: {
                    ImGui::Text("Gameplay Modifications");
                    ImGui::Separator();

                    if (ImGui::Checkbox("God Mode", &Cheat::Config::Features::GodMode)) {
                        LOG_INFO("GUI: God Mode %s", Cheat::Config::Features::GodMode ? "ENABLED" : "DISABLED");
                    }
                    if (ImGui::IsItemHovered()) {
                        ImGui::SetTooltip("Always max health)");
                    }

                    

                    // Movement Hacks Section
                    ImGui::Separator();
                    ImGui::Text("Movement Hacks");
                    ImGui::Separator();

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
                            ImGui::Text("Original speeds: MovementSpeedModifier=%.1f", Cheat::Config::Features::originalMovementSpeedModifier);
                        } else {
                            ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "Original speeds not captured yet");
                        }
                        ImGui::Unindent();
                    }

                    if (ImGui::Checkbox("Slow Immunity", &Cheat::Config::Features::SlowImmunity)) {
                        LOG_INFO("GUI: Slow Immunity %s", Cheat::Config::Features::SlowImmunity ? "ENABLED" : "DISABLED");
                    }
                    if (ImGui::IsItemHovered()) {
                        ImGui::SetTooltip("Immunity to slow effects");
                    }

                    if (ImGui::Checkbox("Jump Height Hack", &Cheat::Config::Features::JumpHeightHack)) {
                        LOG_INFO("GUI: Jump Height Hack %s", Cheat::Config::Features::JumpHeightHack ? "ENABLED" : "DISABLED");
                    }
                    if (ImGui::IsItemHovered()) {
                        ImGui::SetTooltip("Multiply jump height");
                    }

                    // Jump height multiplier slider (only show when jump height hack is enabled)
                    if (Cheat::Config::Features::JumpHeightHack) {
                        ImGui::Indent();
                        if (ImGui::SliderFloat("Jump Height Multiplier", &Cheat::Config::Features::JumpHeightMultiplier, 0.1f, 10.0f, "%.1fx")) {
                            LOG_INFO("GUI: Jump Height Multiplier changed to %.1fx", Cheat::Config::Features::JumpHeightMultiplier);
                        }
                        if (ImGui::IsItemHovered()) {
                            ImGui::SetTooltip("1.0x = normal jump, 2.0x = double jump height, etc.");
                        }
                        ImGui::Unindent();
                    }

                    if (ImGui::Checkbox("Dash Distance", &Cheat::Config::Features::DashSpeedHack)) {
                        LOG_INFO("GUI: Dash Speed Hack %s", Cheat::Config::Features::DashSpeedHack ? "ENABLED" : "DISABLED");
                    }
                    if (ImGui::IsItemHovered()) {
                        ImGui::SetTooltip("Multiply dash distance");
                    }

                    // Dash speed multiplier slider (only show when dash speed hack is enabled)
                    if (Cheat::Config::Features::DashSpeedHack) {
                        ImGui::Indent();
                        if (ImGui::SliderFloat("Dash Distance Multiplier", &Cheat::Config::Features::DashSpeedMultiplier, 0.1f, 10.0f, "%.1fx")) {
                            LOG_INFO("GUI: Dash Speed Multiplier changed to %.1fx", Cheat::Config::Features::DashSpeedMultiplier);
                        }
                        if (ImGui::IsItemHovered()) {
                            ImGui::SetTooltip("1.0x = normal dash, 2.0x = double dash speed/time, etc.");
                        }
                        ImGui::Unindent();
                    }

                    break;
                }

                case MenuTab::OPTIONS: {
                    ImGui::Text("Options & Settings");
                    ImGui::Separator();

                    // GUI Scale Configuration
                    ImGui::Text("Interface Settings");
                    ImGui::Separator();

                    static float previousScale = Cheat::Config::GUI::Scale;
                    if (ImGui::SliderFloat("GUI Scale", &Cheat::Config::GUI::Scale, 0.5f, 3.0f, "%.1fx")) {
                        // Apply the new scale immediately
                        ImGuiIO& io = ImGui::GetIO();
                        io.FontGlobalScale = Cheat::Config::GUI::Scale;

                        // Update window size to match new scale
                        ImGui::SetWindowSize(ImVec2(600 * Cheat::Config::GUI::Scale, 400 * Cheat::Config::GUI::Scale));

                        LOG_INFO("GUI: GUI Scale changed from %.1fx to %.1fx", previousScale, Cheat::Config::GUI::Scale);
                        previousScale = Cheat::Config::GUI::Scale;
                    }
                    if (ImGui::IsItemHovered()) {
                        ImGui::SetTooltip("Adjust the size of the GUI interface\n0.5x = Half size, 1.0x = Normal, 2.0x = Double size");
                    }

                    ImGui::Spacing();
                    ImGui::Text("Current Scale: %.1fx", Cheat::Config::GUI::Scale);

                    // Reset button
                    if (ImGui::Button("Reset to Default (1.0x)")) {
                        Cheat::Config::GUI::Scale = 1.0f;
                        ImGuiIO& io = ImGui::GetIO();
                        io.FontGlobalScale = 1.0f;
                        ImGui::SetWindowSize(ImVec2(600 * 1.0f, 400 * 1.0f));
                        LOG_INFO("GUI: GUI Scale reset to default (1.0x)");
                    }
                    if (ImGui::IsItemHovered()) {
                        ImGui::SetTooltip("Reset GUI scale to normal size");
                    }

                    ImGui::SameLine();
                    if (ImGui::Button("1.5x")) {
                        Cheat::Config::GUI::Scale = 1.5f;
                        ImGuiIO& io = ImGui::GetIO();
                        io.FontGlobalScale = 1.5f;
                        ImGui::SetWindowSize(ImVec2(600 * 1.5f, 400 * 1.5f));
                        LOG_INFO("GUI: GUI Scale set to (1.5x)");
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
                    ImGui::BulletText("F5: Log weapon stats (debug)");
                    ImGui::BulletText("%s: Hold to activate aimbot", Cheat::Utils::Input::GetKeyName(Cheat::Config::Hotkeys::AimbotTrigger));
                    ImGui::BulletText("Insert: Toggle this menu");
                    ImGui::BulletText("F9: Exit cheat system (CAUSES GAME CRASH)");

                    break;
                }

                case MenuTab::DEBUG: {
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
