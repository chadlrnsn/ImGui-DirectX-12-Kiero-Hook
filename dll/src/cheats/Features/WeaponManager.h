#pragma once

#include "../SDK/Engine_classes.hpp"
#include "../SDK/RGame_classes.hpp"
#include "../SDK/BP_EngineRifle_Script_classes.hpp"

namespace Cheat {
    namespace Features {
        
        // Structure to store original weapon settings
        struct OriginalWeaponSettings {
            // Ammo settings
            int32_t BaseAmmoCost = 0;

            // Damage settings
            float BaseWeaponDamage = 0.0f;
            float BaseWeaponDamageMinRange = 0.0f;
            float BaseWeaponDamageMaxRange = 0.0f;

            // Critical hit settings
            float BaseWeaponCriticalMultiplier = 0.0f;
            float BaseWeaponCriticalMultiplierMinRange = 0.0f;
            float BaseWeaponCriticalMultiplierMaxRange = 0.0f;

            // Rate of fire settings
            float BaseRateOfFire = 0.0f;
            float BaseRateOfFireMinRange = 0.0f;
            float BaseRateOfFireMaxRange = 0.0f;

            // Cooldown settings
            float BaseCooldown = 0.0f;
            float BaseCooldownMinRange = 0.0f;
            float BaseCooldownMaxRange = 0.0f;

            // Recoil settings
            float BaseRecoil = 0.0f;
            float BaseRecoilMinRange = 0.0f;
            float BaseRecoilMaxRange = 0.0f;

            // Recoil recovery settings
            float BaseRecoilRecovery = 0.0f;
            float BaseRecoilRecoveryMinRange = 0.0f;
            float BaseRecoilRecoveryMaxRange = 0.0f;

            // Spread settings
            float BaseSpread = 0.0f;
            float BaseSpreadMinRange = 0.0f;
            float BaseSpreadMaxRange = 0.0f;

            // Reload settings
            float BaseReloadTime = 0.0f;
            float BaseReloadTimeMinRange = 0.0f;
            float BaseReloadTimeMaxRange = 0.0f;
            float ReloadTimeDelta = 0.0f;

            bool isValid = false;
        };

        class WeaponManager {
        public:
            // Lifecycle
            static void Initialize();
            static void Update(SDK::ARPlayerPawn* playerPawn);
            static void Shutdown();

            // Weapon modifications
            static void ApplyModifications(SDK::UWorld* world);

            // UI-driven application: call this whenever a checkbox/slider is changed
            static void OnWeaponSettingsChanged();

        private:
            // Cached weapon references
            static SDK::ARPlayerPawn* s_cachedCharacter;
            static SDK::ARWeapon* s_cachedWeapon;
            static SDK::URGWeaponScript* s_cachedWeaponScript;
            static SDK::UBP_EngineRifle_Script_C* s_cachedEngineRifleScript;
            static bool s_isEngineRifle;

            // Original settings storage (primary and secondary stored separately)
            static OriginalWeaponSettings s_originalPrimarySettings;
            static OriginalWeaponSettings s_originalSecondarySettings;
            static bool s_primaryOriginalSaved;
            static bool s_secondaryOriginalSaved;

            // Internal methods
            static void UpdateWeaponCache(SDK::ARPlayerPawn* character);
            static void ManageEngineRifleHeat();
            static void ApplyWeaponSettings(SDK::URGWeaponScript* weaponScript);
            static void RestoreOriginalSettings(SDK::URGWeaponScript* weaponScript);
            static void SaveOriginalSettings(SDK::URGWeaponScript* weaponScript);
            static void PrintWeaponInfo(SDK::ARWeapon* weapon);
            static void ApplyToSecondaryWeapon(SDK::URGWeaponScript* weaponScript, bool restore = false);

            static bool AnyModsEnabled();
        };
        
    } // namespace Features
} // namespace Cheat
