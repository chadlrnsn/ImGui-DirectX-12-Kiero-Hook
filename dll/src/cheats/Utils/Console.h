#pragma once

#include "../SDK/Engine_classes.hpp"
#include "../SDK/RGame_classes.hpp"
#include <string>

namespace Cheat {
    namespace Utils {
        
        class Console {
        public:
            // Console output
            static void Write(const std::string& text);
            static void Write(const std::wstring& text);
            static void DebugPrint(const std::string& text);
            
            // Cheat manager
            static bool EnableCheatManager(SDK::APlayerController* playerController);
            
            // UI
            static void PrintControls();
            
        private:
            static bool s_autoCheatManager;
        };
        
    } // namespace Utils
} // namespace Cheat
