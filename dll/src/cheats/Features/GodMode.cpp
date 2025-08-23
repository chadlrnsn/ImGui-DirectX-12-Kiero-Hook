#include "GodMode.h"
#include "../Core/Config.h"
#include <dev/logger.h>

namespace Cheat {
    namespace Features {
        
        bool GodMode::s_enabled = true;
        
        void GodMode::Initialize() {
            LOG_INFO("GodMode initialized (enabled by default)");
        }
        
        void GodMode::Update(SDK::ARPlayerPawn* playerPawn) {
            if (!Cheat::Config::Features::GodMode || !playerPawn) {
                return;
            }

            // Keep player health at max
            if (playerPawn->HealthComponent) {
                playerPawn->HealthComponent->currentHealth = playerPawn->HealthComponent->GetMaxHealth();
            }
        }
        
        void GodMode::Shutdown() {
            LOG_INFO("GodMode shutdown");
        }
        
    } // namespace Features
} // namespace Cheat
