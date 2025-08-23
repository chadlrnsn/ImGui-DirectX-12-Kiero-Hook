#pragma once

#include "../SDK/Engine_classes.hpp"
#include "../SDK/RGame_classes.hpp"

namespace Cheat {
    namespace Features {
        
        class GodMode {
        public:
            // Lifecycle
            static void Initialize();
            static void Update(SDK::ARPlayerPawn* playerPawn);
            static void Shutdown();
            
            // Control
            static void Enable() { s_enabled = true; }
            static void Disable() { s_enabled = false; }
            static void Toggle() { s_enabled = !s_enabled; }
            static bool IsEnabled() { return s_enabled; }
            
        private:
            static bool s_enabled;
        };
        
    } // namespace Features
} // namespace Cheat
