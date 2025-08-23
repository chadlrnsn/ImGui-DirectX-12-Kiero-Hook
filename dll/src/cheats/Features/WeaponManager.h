#pragma once

#include "../SDK/Engine_classes.hpp"
#include "../SDK/RGame_classes.hpp"
#include "../SDK/BP_EngineRifle_Script_classes.hpp"

namespace Cheat {
    namespace Features {
        
        class WeaponManager {
        public:
            // Lifecycle
            static void Initialize();
            static void Update(SDK::ARPlayerPawn* playerPawn);
            static void Shutdown();
            
            // Weapon modifications
            static void ApplyModifications(SDK::UWorld* world);
            
        private:
            // Cached weapon references
            static SDK::ARPlayerPawn* s_cachedCharacter;
            static SDK::ARWeapon* s_cachedWeapon;
            static SDK::URGWeaponScript* s_cachedWeaponScript;
            static SDK::UBP_EngineRifle_Script_C* s_cachedEngineRifleScript;
            static bool s_isEngineRifle;
            
            // Internal methods
            static void UpdateWeaponCache(SDK::ARPlayerPawn* character);
            static void ManageEngineRifleHeat();
            static void ApplyWeaponSettings(SDK::URGWeaponScript* weaponScript);
            static void PrintWeaponInfo(SDK::ARWeapon* weapon);
        };
        
    } // namespace Features
} // namespace Cheat
