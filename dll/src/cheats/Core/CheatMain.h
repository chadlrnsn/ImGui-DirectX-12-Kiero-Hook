#pragma once

#include "../SDK/Engine_classes.hpp"
#include "../SDK/RGame_classes.hpp"
#include "Config.h"

namespace Cheat {
    namespace Core {
        
        class CheatMain {
        public:
            // Core lifecycle methods
            static bool Initialize();
            static void Update(DWORD tick);
            static void Shutdown();

            // State queries
            static bool IsInitialized() { return Config::System::Initialized; }
            static bool ShouldExit() { return Config::System::ShouldExit; }

            // System control
            static void RequestExit() { Config::System::ShouldExit = true; }

        private:
            // SDK update and entity fetching (like your example)
            static bool UpdateSDK(bool log = false);
            static void FetchEntities();
            static void FetchFromActors(std::vector<SDK::AActor*>* list);

            // Initialization helpers
            static bool InitializeSubsystems();

            // Update helpers
            static void ProcessInput();
            static void UpdateSubsystems(float deltaTime);
        };
        
    } // namespace Core
} // namespace Cheat
