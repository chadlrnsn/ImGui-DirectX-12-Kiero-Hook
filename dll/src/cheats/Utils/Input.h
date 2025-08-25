#pragma once

#include <Windows.h>
#include <unordered_map>

namespace Cheat {
    namespace Utils {
        
        class Input {
        public:
            // Key state management
            static bool IsKeyPressed(int vkCode);
            static bool IsKeyDown(int vkCode);
            static bool IsKeyUp(int vkCode);

            // Update key states (call once per frame)
            static void Update();

            // Hotkey configuration utilities
            static const char* GetKeyName(BYTE vkCode);
            static BYTE CaptureNextKeyPress();  // Returns 0 if no key pressed, VK_ESCAPE to cancel

        private:
            static std::unordered_map<int, bool> s_previousKeyStates;
            static std::unordered_map<int, bool> s_currentKeyStates;

            static bool GetKeyState(int vkCode);
        };
        
    } // namespace Utils
} // namespace Cheat
