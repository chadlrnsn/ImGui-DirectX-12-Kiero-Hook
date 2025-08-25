#pragma once

namespace CheatMenu {
    
    enum class MenuTab {
        AIMBOT = 0,
        WEAPONS,
        GAMEPLAY,
        UNLOCKS,
        OPTIONS,
        DEBUG
    };
    
    // Toggle menu visibility
    void Toggle();
    
    // Check if menu is visible
    bool IsVisible();
    
    // Render the cheat menu
    void Render();
    
} // namespace CheatMenu
