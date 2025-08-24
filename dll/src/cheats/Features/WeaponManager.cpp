#include "WeaponManager.h"
#include "../Core/Config.h"
#include "../Utils/Console.h"
#include <dev/logger.h>

namespace Cheat {
    namespace Features {

        // Static member definitions
        SDK::ARPlayerPawn* WeaponManager::s_cachedCharacter = nullptr;
        SDK::ARWeapon* WeaponManager::s_cachedWeapon = nullptr;
        SDK::URGWeaponScript* WeaponManager::s_cachedWeaponScript = nullptr;
        SDK::UBP_EngineRifle_Script_C* WeaponManager::s_cachedEngineRifleScript = nullptr;
        bool WeaponManager::s_isEngineRifle = false;
        OriginalWeaponSettings WeaponManager::s_originalPrimarySettings;
        OriginalWeaponSettings WeaponManager::s_originalSecondarySettings;
        bool WeaponManager::s_primaryOriginalSaved = false;
        bool WeaponManager::s_secondaryOriginalSaved = false;

        void WeaponManager::Initialize() {
            LOG_INFO("WeaponManager initialized");
        }

        void WeaponManager::OnWeaponSettingsChanged() {
            // Called from UI on any toggle/slider change
            if (!s_cachedWeaponScript) return;

            const bool anyEnabled = AnyModsEnabled();

            // Save originals lazily per-weapon when enabling mods
            if (anyEnabled && !s_primaryOriginalSaved) {
                SaveOriginalSettings(s_cachedWeaponScript);
                s_primaryOriginalSaved = true;
            }

            // For secondary, just mark saved flag when we first apply to it; its values come from primary base settings via ApplyFireSettings
            if (anyEnabled && s_isEngineRifle && s_cachedEngineRifleScript && s_cachedEngineRifleScript->SecondaryWeaponModScript && !s_secondaryOriginalSaved) {
                s_secondaryOriginalSaved = true;
            }

            if (anyEnabled) {
                ApplyWeaponSettings(s_cachedWeaponScript);
            } else {
                // Restore and clear saved flags
                RestoreOriginalSettings(s_cachedWeaponScript);
                s_primaryOriginalSaved = false;
                s_secondaryOriginalSaved = false;
            }
        }

        bool WeaponManager::AnyModsEnabled() {
            using namespace Cheat::Config::Features;
            return InfiniteAmmo || IncreasedDamage || HighCritMultiplier || FastRateOfFire || NoCooldown || NoRecoil || InstantReload || RateOfFireOverride;
        }

        void WeaponManager::Update(SDK::ARPlayerPawn* playerPawn) {
            UpdateWeaponCache(playerPawn);
            if (Cheat::Config::Features::EngineRifleHeatManagement) {
                ManageEngineRifleHeat();
            }

            // Engine heat management happens every frame; weapon settings no longer tick-apply
            if (s_cachedWeaponScript) {
                // Nothing else here; weapon settings are applied immediately via OnWeaponSettingsChanged()
            }
        }

        void WeaponManager::Shutdown() {
            s_cachedCharacter = nullptr;
            s_cachedWeapon = nullptr;
            s_cachedWeaponScript = nullptr;
            s_cachedEngineRifleScript = nullptr;
            s_isEngineRifle = false;
            s_originalPrimarySettings = OriginalWeaponSettings();
            s_originalSecondarySettings = OriginalWeaponSettings();
            s_primaryOriginalSaved = false;
            s_secondaryOriginalSaved = false;
            LOG_INFO("WeaponManager shutdown");
        }

        void WeaponManager::ApplyModifications(SDK::UWorld* world) {
            LOG_INFO("=== F1 PRESSED - APPLYING WEAPON MODIFICATIONS ===");

            auto playerController = world->OwningGameInstance->LocalPlayers[0]->PlayerController;
            if (!playerController) {
                LOG_ERROR("PlayerController not found");
                return;
            }

            auto character = static_cast<SDK::ARPlayerPawn*>(playerController->K2_GetPawn());
            if (!character) {
                LOG_ERROR("Player Pawn not spawned yet");
                return;
            }

            auto weapon = character->GetEquippedWeapon();
            if (!weapon) {
                LOG_ERROR("Player weapon not found");
                return;
            }

            PrintWeaponInfo(weapon);

            auto weaponScript = weapon->RuntimeWeaponScript;
            if (!weaponScript) {
                LOG_ERROR("Weapon Runtime Script not found");
                return;
            }

            ApplyWeaponSettings(weaponScript);
            LOG_INFO("Weapon modifications applied successfully!");
        }

        void WeaponManager::UpdateWeaponCache(SDK::ARPlayerPawn* character) {
            if (!character) {
                // Clear both local and global caches
                s_cachedCharacter = nullptr;
                s_cachedWeapon = nullptr;
                s_cachedWeaponScript = nullptr;
                s_cachedEngineRifleScript = nullptr;
                s_isEngineRifle = false;

                Cheat::Config::GameState::g_pCachedPlayerPawn = nullptr;
                Cheat::Config::GameState::g_pCachedWeapon = nullptr;
                Cheat::Config::GameState::g_pCachedWeaponScript = nullptr;
                Cheat::Config::GameState::g_pCachedEngineRifleScript = nullptr;
                Cheat::Config::GameState::g_bIsEngineRifle = false;
                return;
            }
            // Update both local and global character cache
            s_cachedCharacter = character;
            Cheat::Config::GameState::g_pCachedPlayerPawn = character;

            // Update weapon cache
            auto weapon = character->GetEquippedWeapon();
            if (weapon != s_cachedWeapon) {
                s_cachedWeapon = weapon;
                s_cachedWeaponScript = nullptr;
                s_cachedEngineRifleScript = nullptr;
                s_isEngineRifle = false;

                // Reset original settings when weapon changes
                s_originalPrimarySettings = OriginalWeaponSettings();
                s_originalSecondarySettings = OriginalWeaponSettings();
                s_primaryOriginalSaved = false;
                s_secondaryOriginalSaved = false;

                // Update global cache
                Cheat::Config::GameState::g_pCachedWeapon = weapon;
                Cheat::Config::GameState::g_pCachedWeaponScript = nullptr;
                Cheat::Config::GameState::g_pCachedEngineRifleScript = nullptr;
                Cheat::Config::GameState::g_bIsEngineRifle = false;

                if (weapon && weapon->RuntimeWeaponScript) {
                    s_cachedWeaponScript = weapon->RuntimeWeaponScript;
                    Cheat::Config::GameState::g_pCachedWeaponScript = weapon->RuntimeWeaponScript;

                    // Check if it's an Engine Rifle and cache the specialized script
                    if (s_cachedWeaponScript->IsA(SDK::UBP_EngineRifle_Script_C::StaticClass())) {
                        s_cachedEngineRifleScript = static_cast<SDK::UBP_EngineRifle_Script_C*>(s_cachedWeaponScript);
                        s_isEngineRifle = true;

                        Cheat::Config::GameState::g_pCachedEngineRifleScript = s_cachedEngineRifleScript;
                        Cheat::Config::GameState::g_bIsEngineRifle = true;

                        LOG_INFO("[WEAPON] Engine Rifle detected and cached!");
                    }
                }
            }
        }

        void WeaponManager::ManageEngineRifleHeat() {
            if (s_isEngineRifle && s_cachedEngineRifleScript) {
                // Force set heat to 0 to prevent overheating
                s_cachedEngineRifleScript->ForceSetHeat(0.0);
            }
        }

        static float Clampf(float v, float minv, float maxv) {
            if (v < minv) return minv; if (v > maxv) return maxv; return v;
        }

        void WeaponManager::ApplyWeaponSettings(SDK::URGWeaponScript* weaponScript) {
            using namespace Cheat::Config::Features;

            auto primary = weaponScript->GetBaseWeaponSettings();
            if (!primary) {
                LOG_ERROR("Failed to get primary weapon settings");
                return;
            }

            // Ensure originals exist if applying due to hotkeys instead of UI callback
            if (!s_primaryOriginalSaved && AnyModsEnabled()) {
                SaveOriginalSettings(weaponScript);
                s_primaryOriginalSaved = true;
            }

            // --- PRIMARY ---
            // Ammo
            primary->BaseAmmoCost.BaseValue = InfiniteAmmo ? 0 : s_originalPrimarySettings.BaseAmmoCost;

            // Damage (multiplier)
            if (IncreasedDamage) {
                const float minV = s_originalPrimarySettings.BaseWeaponDamageMinRange;
                const float maxV = s_originalPrimarySettings.BaseWeaponDamageMaxRange;
                primary->BaseWeaponDamage.BaseValue = Clampf(s_originalPrimarySettings.BaseWeaponDamage * DamageMultiplier, minV, maxV);
            } else {
                primary->BaseWeaponDamage.BaseValue = s_originalPrimarySettings.BaseWeaponDamage;
            }
            primary->BaseWeaponDamage.MinMaxRange.X = s_originalPrimarySettings.BaseWeaponDamageMinRange;
            primary->BaseWeaponDamage.MinMaxRange.Y = s_originalPrimarySettings.BaseWeaponDamageMaxRange;

            // Crit (multiplier)
            if (HighCritMultiplier) {
                const float minV = s_originalPrimarySettings.BaseWeaponCriticalMultiplierMinRange;
                const float maxV = s_originalPrimarySettings.BaseWeaponCriticalMultiplierMaxRange;
                primary->BaseWeaponCriticalMultiplier.BaseValue = Clampf(s_originalPrimarySettings.BaseWeaponCriticalMultiplier * CritMultiplier, minV, maxV);
            } else {
                primary->BaseWeaponCriticalMultiplier.BaseValue = s_originalPrimarySettings.BaseWeaponCriticalMultiplier;
            }
            primary->BaseWeaponCriticalMultiplier.MinMaxRange.X = s_originalPrimarySettings.BaseWeaponCriticalMultiplierMinRange;
            primary->BaseWeaponCriticalMultiplier.MinMaxRange.Y = s_originalPrimarySettings.BaseWeaponCriticalMultiplierMaxRange;

            // Rate of fire: either override slider or fast toggle, else original
            if (RateOfFireOverride) {
                float minV = s_originalPrimarySettings.BaseRateOfFireMinRange;
                float maxV = s_originalPrimarySettings.BaseRateOfFireMaxRange;
                float v = RateOfFireValue;
                if (v <= 0.0f) v = s_originalPrimarySettings.BaseRateOfFire; // default to original when unset
                primary->BaseRateOfFire.BaseValue = Clampf(v, minV, maxV);
                primary->BaseRateOfFire.MinMaxRange.X = minV;
                primary->BaseRateOfFire.MinMaxRange.Y = maxV;
            } else if (FastRateOfFire) {
                primary->BaseRateOfFire.BaseValue = s_originalPrimarySettings.BaseRateOfFireMaxRange;
                primary->BaseRateOfFire.MinMaxRange.X = s_originalPrimarySettings.BaseRateOfFireMinRange;
                primary->BaseRateOfFire.MinMaxRange.Y = s_originalPrimarySettings.BaseRateOfFireMaxRange;
            } else {
                primary->BaseRateOfFire.BaseValue = s_originalPrimarySettings.BaseRateOfFire;
                primary->BaseRateOfFire.MinMaxRange.X = s_originalPrimarySettings.BaseRateOfFireMinRange;
                primary->BaseRateOfFire.MinMaxRange.Y = s_originalPrimarySettings.BaseRateOfFireMaxRange;
            }

            // Cooldown
            if (NoCooldown) {
                primary->BaseCooldown.BaseValue = 0.0f;
                primary->BaseCooldown.MinMaxRange.X = 0.0f;
                primary->BaseCooldown.MinMaxRange.Y = 0.0f;
            } else {
                primary->BaseCooldown.BaseValue = s_originalPrimarySettings.BaseCooldown;
                primary->BaseCooldown.MinMaxRange.X = s_originalPrimarySettings.BaseCooldownMinRange;
                primary->BaseCooldown.MinMaxRange.Y = s_originalPrimarySettings.BaseCooldownMaxRange;
            }

            // Recoil + Recovery + Spread
            if (NoRecoil) {
                primary->BaseRecoil.BaseValue = 0.0f;
                primary->BaseRecoil.MinMaxRange.X = 0.0f;
                primary->BaseRecoil.MinMaxRange.Y = 0.0f;
                primary->BaseRecoilRecovery.BaseValue = 100.0f;
                primary->BaseRecoilRecovery.MinMaxRange.X = 0.0f;
                primary->BaseRecoilRecovery.MinMaxRange.Y = 9999.0f;
                primary->BaseSpread.BaseValue = 0.0f;
                primary->BaseSpread.MinMaxRange.X = 0.0f;
                primary->BaseSpread.MinMaxRange.Y = 0.0f;
            } else {
                primary->BaseRecoil.BaseValue = s_originalPrimarySettings.BaseRecoil;
                primary->BaseRecoil.MinMaxRange.X = s_originalPrimarySettings.BaseRecoilMinRange;
                primary->BaseRecoil.MinMaxRange.Y = s_originalPrimarySettings.BaseRecoilMaxRange;
                primary->BaseRecoilRecovery.BaseValue = s_originalPrimarySettings.BaseRecoilRecovery;
                primary->BaseRecoilRecovery.MinMaxRange.X = s_originalPrimarySettings.BaseRecoilRecoveryMinRange;
                primary->BaseRecoilRecovery.MinMaxRange.Y = s_originalPrimarySettings.BaseRecoilRecoveryMaxRange;
                primary->BaseSpread.BaseValue = s_originalPrimarySettings.BaseSpread;
                primary->BaseSpread.MinMaxRange.X = s_originalPrimarySettings.BaseSpreadMinRange;
                primary->BaseSpread.MinMaxRange.Y = s_originalPrimarySettings.BaseSpreadMaxRange;
            }

            // Reload
            if (InstantReload) {
                primary->BaseReloadTime.BaseValue = 0.01f;
                primary->BaseReloadTime.MinMaxRange.X = 0.01f;
                primary->BaseReloadTime.MinMaxRange.Y = 0.01f;
                primary->ReloadTimeDelta = 0.0f;
            } else {
                primary->BaseReloadTime.BaseValue = s_originalPrimarySettings.BaseReloadTime;
                primary->BaseReloadTime.MinMaxRange.X = s_originalPrimarySettings.BaseReloadTimeMinRange;
                primary->BaseReloadTime.MinMaxRange.Y = s_originalPrimarySettings.BaseReloadTimeMaxRange;
                primary->ReloadTimeDelta = s_originalPrimarySettings.ReloadTimeDelta;
            }

            // Apply to primary
            weaponScript->SetBaseWeaponSettings(primary);

            // --- SECONDARY (if exists) ---
            if (s_isEngineRifle && s_cachedEngineRifleScript && s_cachedEngineRifleScript->SecondaryWeaponModScript) {
                auto secondary = s_cachedEngineRifleScript->SecondaryWeaponModScript->WeaponModStats;
                if (secondary) {
                    // Apply to secondary
                    s_cachedEngineRifleScript->SecondaryWeaponModScript->ApplyFireSettings(primary);
                }
            }
        }

        void WeaponManager::LogAllWeaponStats() {
            if (!s_cachedWeaponScript) {
                LOG_INFO("=== WEAPON STATS DEBUG ===");
                LOG_INFO("No cached weapon script available");
                return;
            }

            LOG_INFO("=== WEAPON STATS DEBUG ===");

            // PRIMARY WEAPON STATS
            auto primary = s_cachedWeaponScript->GetBaseWeaponSettings();
            if (primary) {
                LOG_INFO("--- PRIMARY WEAPON ---");
                LOG_INFO("Ammo Cost: %d", primary->BaseAmmoCost.BaseValue);
                LOG_INFO("Damage: %.2f (Range: %.2f - %.2f)",
                    primary->BaseWeaponDamage.BaseValue,
                    primary->BaseWeaponDamage.MinMaxRange.X,
                    primary->BaseWeaponDamage.MinMaxRange.Y);
                LOG_INFO("Crit Multiplier: %.2f (Range: %.2f - %.2f)",
                    primary->BaseWeaponCriticalMultiplier.BaseValue,
                    primary->BaseWeaponCriticalMultiplier.MinMaxRange.X,
                    primary->BaseWeaponCriticalMultiplier.MinMaxRange.Y);
                LOG_INFO("Rate of Fire: %.2f (Range: %.2f - %.2f)",
                    primary->BaseRateOfFire.BaseValue,
                    primary->BaseRateOfFire.MinMaxRange.X,
                    primary->BaseRateOfFire.MinMaxRange.Y);
                LOG_INFO("Cooldown: %.2f (Range: %.2f - %.2f)",
                    primary->BaseCooldown.BaseValue,
                    primary->BaseCooldown.MinMaxRange.X,
                    primary->BaseCooldown.MinMaxRange.Y);
                LOG_INFO("Recoil: %.2f (Range: %.2f - %.2f)",
                    primary->BaseRecoil.BaseValue,
                    primary->BaseRecoil.MinMaxRange.X,
                    primary->BaseRecoil.MinMaxRange.Y);
                LOG_INFO("Recoil Recovery: %.2f (Range: %.2f - %.2f)",
                    primary->BaseRecoilRecovery.BaseValue,
                    primary->BaseRecoilRecovery.MinMaxRange.X,
                    primary->BaseRecoilRecovery.MinMaxRange.Y);
                LOG_INFO("Spread: %.2f (Range: %.2f - %.2f)",
                    primary->BaseSpread.BaseValue,
                    primary->BaseSpread.MinMaxRange.X,
                    primary->BaseSpread.MinMaxRange.Y);
                LOG_INFO("Reload Time: %.2f (Range: %.2f - %.2f), Delta: %.2f",
                    primary->BaseReloadTime.BaseValue,
                    primary->BaseReloadTime.MinMaxRange.X,
                    primary->BaseReloadTime.MinMaxRange.Y,
                    primary->ReloadTimeDelta);
            } else {
                LOG_INFO("--- PRIMARY WEAPON ---");
                LOG_INFO("Failed to get primary weapon settings");
            }

            // SECONDARY WEAPON STATS (if Engine Rifle)
            if (s_isEngineRifle && s_cachedEngineRifleScript && s_cachedEngineRifleScript->SecondaryWeaponModScript) {
                auto secondary = s_cachedEngineRifleScript->SecondaryWeaponModScript->WeaponModStats;
                if (secondary) {
                    LOG_INFO("--- SECONDARY WEAPON ---");
                    LOG_INFO("Ammo Cost: %d", secondary->BaseAmmoCost.BaseValue);
                    LOG_INFO("Damage: %.2f (Range: %.2f - %.2f)",
                        secondary->BaseWeaponDamage.BaseValue,
                        secondary->BaseWeaponDamage.MinMaxRange.X,
                        secondary->BaseWeaponDamage.MinMaxRange.Y);
                    LOG_INFO("Crit Multiplier: %.2f (Range: %.2f - %.2f)",
                        secondary->BaseWeaponCriticalMultiplier.BaseValue,
                        secondary->BaseWeaponCriticalMultiplier.MinMaxRange.X,
                        secondary->BaseWeaponCriticalMultiplier.MinMaxRange.Y);
                    LOG_INFO("Rate of Fire: %.2f (Range: %.2f - %.2f)",
                        secondary->BaseRateOfFire.BaseValue,
                        secondary->BaseRateOfFire.MinMaxRange.X,
                        secondary->BaseRateOfFire.MinMaxRange.Y);
                    LOG_INFO("Cooldown: %.2f (Range: %.2f - %.2f)",
                        secondary->BaseCooldown.BaseValue,
                        secondary->BaseCooldown.MinMaxRange.X,
                        secondary->BaseCooldown.MinMaxRange.Y);
                    LOG_INFO("Recoil: %.2f (Range: %.2f - %.2f)",
                        secondary->BaseRecoil.BaseValue,
                        secondary->BaseRecoil.MinMaxRange.X,
                        secondary->BaseRecoil.MinMaxRange.Y);
                    LOG_INFO("Recoil Recovery: %.2f (Range: %.2f - %.2f)",
                        secondary->BaseRecoilRecovery.BaseValue,
                        secondary->BaseRecoilRecovery.MinMaxRange.X,
                        secondary->BaseRecoilRecovery.MinMaxRange.Y);
                    LOG_INFO("Spread: %.2f (Range: %.2f - %.2f)",
                        secondary->BaseSpread.BaseValue,
                        secondary->BaseSpread.MinMaxRange.X,
                        secondary->BaseSpread.MinMaxRange.Y);
                    LOG_INFO("Reload Time: %.2f (Range: %.2f - %.2f), Delta: %.2f",
                        secondary->BaseReloadTime.BaseValue,
                        secondary->BaseReloadTime.MinMaxRange.X,
                        secondary->BaseReloadTime.MinMaxRange.Y,
                        secondary->ReloadTimeDelta);
                } else {
                    LOG_INFO("--- SECONDARY WEAPON ---");
                    LOG_INFO("Failed to get secondary weapon settings");
                }
            } else {
                LOG_INFO("--- SECONDARY WEAPON ---");
                LOG_INFO("Not an Engine Rifle or no secondary weapon mod script");
            }

            // ORIGINAL SETTINGS (if saved)
            if (s_primaryOriginalSaved) {
                LOG_INFO("--- ORIGINAL PRIMARY SETTINGS (SAVED) ---");
                LOG_INFO("Ammo Cost: %d", s_originalPrimarySettings.BaseAmmoCost);
                LOG_INFO("Damage: %.2f (Range: %.2f - %.2f)",
                    s_originalPrimarySettings.BaseWeaponDamage,
                    s_originalPrimarySettings.BaseWeaponDamageMinRange,
                    s_originalPrimarySettings.BaseWeaponDamageMaxRange);
                LOG_INFO("Crit Multiplier: %.2f (Range: %.2f - %.2f)",
                    s_originalPrimarySettings.BaseWeaponCriticalMultiplier,
                    s_originalPrimarySettings.BaseWeaponCriticalMultiplierMinRange,
                    s_originalPrimarySettings.BaseWeaponCriticalMultiplierMaxRange);
                LOG_INFO("Rate of Fire: %.2f (Range: %.2f - %.2f)",
                    s_originalPrimarySettings.BaseRateOfFire,
                    s_originalPrimarySettings.BaseRateOfFireMinRange,
                    s_originalPrimarySettings.BaseRateOfFireMaxRange);
                LOG_INFO("Cooldown: %.2f (Range: %.2f - %.2f)",
                    s_originalPrimarySettings.BaseCooldown,
                    s_originalPrimarySettings.BaseCooldownMinRange,
                    s_originalPrimarySettings.BaseCooldownMaxRange);
                LOG_INFO("Recoil: %.2f (Range: %.2f - %.2f)",
                    s_originalPrimarySettings.BaseRecoil,
                    s_originalPrimarySettings.BaseRecoilMinRange,
                    s_originalPrimarySettings.BaseRecoilMaxRange);
                LOG_INFO("Recoil Recovery: %.2f (Range: %.2f - %.2f)",
                    s_originalPrimarySettings.BaseRecoilRecovery,
                    s_originalPrimarySettings.BaseRecoilRecoveryMinRange,
                    s_originalPrimarySettings.BaseRecoilRecoveryMaxRange);
                LOG_INFO("Spread: %.2f (Range: %.2f - %.2f)",
                    s_originalPrimarySettings.BaseSpread,
                    s_originalPrimarySettings.BaseSpreadMinRange,
                    s_originalPrimarySettings.BaseSpreadMaxRange);
                LOG_INFO("Reload Time: %.2f (Range: %.2f - %.2f), Delta: %.2f",
                    s_originalPrimarySettings.BaseReloadTime,
                    s_originalPrimarySettings.BaseReloadTimeMinRange,
                    s_originalPrimarySettings.BaseReloadTimeMaxRange,
                    s_originalPrimarySettings.ReloadTimeDelta);
            } else {
                LOG_INFO("--- ORIGINAL PRIMARY SETTINGS ---");
                LOG_INFO("Not saved yet");
            }

            LOG_INFO("=== END WEAPON STATS DEBUG ===");
        }

        void WeaponManager::SaveOriginalSettings(SDK::URGWeaponScript* weaponScript) {
            auto ws = weaponScript->GetBaseWeaponSettings();
            if (!ws) {
                LOG_ERROR("Failed to get weapon settings for saving originals");
                return;
            }

            LOG_INFO("Saving original PRIMARY weapon settings...");

            // Save ammo settings
            s_originalPrimarySettings.BaseAmmoCost = ws->BaseAmmoCost.BaseValue;

            // Damage
            s_originalPrimarySettings.BaseWeaponDamage = ws->BaseWeaponDamage.BaseValue;
            s_originalPrimarySettings.BaseWeaponDamageMinRange = ws->BaseWeaponDamage.MinMaxRange.X;
            s_originalPrimarySettings.BaseWeaponDamageMaxRange = ws->BaseWeaponDamage.MinMaxRange.Y;

            // Crit
            s_originalPrimarySettings.BaseWeaponCriticalMultiplier = ws->BaseWeaponCriticalMultiplier.BaseValue;
            s_originalPrimarySettings.BaseWeaponCriticalMultiplierMinRange = ws->BaseWeaponCriticalMultiplier.MinMaxRange.X;
            s_originalPrimarySettings.BaseWeaponCriticalMultiplierMaxRange = ws->BaseWeaponCriticalMultiplier.MinMaxRange.Y;

            // RoF
            s_originalPrimarySettings.BaseRateOfFire = ws->BaseRateOfFire.BaseValue;
            s_originalPrimarySettings.BaseRateOfFireMinRange = ws->BaseRateOfFire.MinMaxRange.X;
            s_originalPrimarySettings.BaseRateOfFireMaxRange = ws->BaseRateOfFire.MinMaxRange.Y;

            // Cooldown
            s_originalPrimarySettings.BaseCooldown = ws->BaseCooldown.BaseValue;
            s_originalPrimarySettings.BaseCooldownMinRange = ws->BaseCooldown.MinMaxRange.X;
            s_originalPrimarySettings.BaseCooldownMaxRange = ws->BaseCooldown.MinMaxRange.Y;

            // Recoil
            s_originalPrimarySettings.BaseRecoil = ws->BaseRecoil.BaseValue;
            s_originalPrimarySettings.BaseRecoilMinRange = ws->BaseRecoil.MinMaxRange.X;
            s_originalPrimarySettings.BaseRecoilMaxRange = ws->BaseRecoil.MinMaxRange.Y;

            // Recoil Recovery
            s_originalPrimarySettings.BaseRecoilRecovery = ws->BaseRecoilRecovery.BaseValue;
            s_originalPrimarySettings.BaseRecoilRecoveryMinRange = ws->BaseRecoilRecovery.MinMaxRange.X;
            s_originalPrimarySettings.BaseRecoilRecoveryMaxRange = ws->BaseRecoilRecovery.MinMaxRange.Y;

            // Spread
            s_originalPrimarySettings.BaseSpread = ws->BaseSpread.BaseValue;
            s_originalPrimarySettings.BaseSpreadMinRange = ws->BaseSpread.MinMaxRange.X;
            s_originalPrimarySettings.BaseSpreadMaxRange = ws->BaseSpread.MinMaxRange.Y;

            // Reload
            s_originalPrimarySettings.BaseReloadTime = ws->BaseReloadTime.BaseValue;
            s_originalPrimarySettings.BaseReloadTimeMinRange = ws->BaseReloadTime.MinMaxRange.X;
            s_originalPrimarySettings.BaseReloadTimeMaxRange = ws->BaseReloadTime.MinMaxRange.Y;
            s_originalPrimarySettings.ReloadTimeDelta = ws->ReloadTimeDelta;

            s_originalPrimarySettings.isValid = true;
        }

        void WeaponManager::RestoreOriginalSettings(SDK::URGWeaponScript* weaponScript) {
            if (!s_originalPrimarySettings.isValid) {
                LOG_ERROR("No valid original PRIMARY settings to restore");
                return;
            }

            auto primary = weaponScript->GetBaseWeaponSettings();
            if (!primary) {
                LOG_ERROR("Failed to get primary weapon settings for restoration");
                return;
            }

            LOG_INFO("Restoring original primary/secondary weapon settings...");

            // PRIMARY
            primary->BaseAmmoCost.BaseValue = s_originalPrimarySettings.BaseAmmoCost;
            primary->BaseWeaponDamage.BaseValue = s_originalPrimarySettings.BaseWeaponDamage;
            primary->BaseWeaponDamage.MinMaxRange.X = s_originalPrimarySettings.BaseWeaponDamageMinRange;
            primary->BaseWeaponDamage.MinMaxRange.Y = s_originalPrimarySettings.BaseWeaponDamageMaxRange;
            primary->BaseWeaponCriticalMultiplier.BaseValue = s_originalPrimarySettings.BaseWeaponCriticalMultiplier;
            primary->BaseWeaponCriticalMultiplier.MinMaxRange.X = s_originalPrimarySettings.BaseWeaponCriticalMultiplierMinRange;
            primary->BaseWeaponCriticalMultiplier.MinMaxRange.Y = s_originalPrimarySettings.BaseWeaponCriticalMultiplierMaxRange;
            primary->BaseRateOfFire.BaseValue = s_originalPrimarySettings.BaseRateOfFire;
            primary->BaseRateOfFire.MinMaxRange.X = s_originalPrimarySettings.BaseRateOfFireMinRange;
            primary->BaseRateOfFire.MinMaxRange.Y = s_originalPrimarySettings.BaseRateOfFireMaxRange;
            primary->BaseCooldown.BaseValue = s_originalPrimarySettings.BaseCooldown;
            primary->BaseCooldown.MinMaxRange.X = s_originalPrimarySettings.BaseCooldownMinRange;
            primary->BaseCooldown.MinMaxRange.Y = s_originalPrimarySettings.BaseCooldownMaxRange;
            primary->BaseRecoil.BaseValue = s_originalPrimarySettings.BaseRecoil;
            primary->BaseRecoil.MinMaxRange.X = s_originalPrimarySettings.BaseRecoilMinRange;
            primary->BaseRecoil.MinMaxRange.Y = s_originalPrimarySettings.BaseRecoilMaxRange;
            primary->BaseRecoilRecovery.BaseValue = s_originalPrimarySettings.BaseRecoilRecovery;
            primary->BaseRecoilRecovery.MinMaxRange.X = s_originalPrimarySettings.BaseRecoilRecoveryMinRange;
            primary->BaseRecoilRecovery.MinMaxRange.Y = s_originalPrimarySettings.BaseRecoilRecoveryMaxRange;
            primary->BaseSpread.BaseValue = s_originalPrimarySettings.BaseSpread;
            primary->BaseSpread.MinMaxRange.X = s_originalPrimarySettings.BaseSpreadMinRange;
            primary->BaseSpread.MinMaxRange.Y = s_originalPrimarySettings.BaseSpreadMaxRange;
            primary->BaseReloadTime.BaseValue = s_originalPrimarySettings.BaseReloadTime;
            primary->BaseReloadTime.MinMaxRange.X = s_originalPrimarySettings.BaseReloadTimeMinRange;
            primary->BaseReloadTime.MinMaxRange.Y = s_originalPrimarySettings.BaseReloadTimeMaxRange;
            primary->ReloadTimeDelta = s_originalPrimarySettings.ReloadTimeDelta;

            weaponScript->SetBaseWeaponSettings(primary);

            // SECONDARY
            if (s_isEngineRifle && s_cachedEngineRifleScript && s_cachedEngineRifleScript->SecondaryWeaponModScript) {
                auto secondary = s_cachedEngineRifleScript->SecondaryWeaponModScript->WeaponModStats;
                if (secondary && s_originalSecondarySettings.isValid) {
                    secondary->BaseAmmoCost.BaseValue = s_originalSecondarySettings.BaseAmmoCost;
                    secondary->BaseWeaponDamage.BaseValue = s_originalSecondarySettings.BaseWeaponDamage;
                    secondary->BaseWeaponDamage.MinMaxRange.X = s_originalSecondarySettings.BaseWeaponDamageMinRange;
                    secondary->BaseWeaponDamage.MinMaxRange.Y = s_originalSecondarySettings.BaseWeaponDamageMaxRange;
                    secondary->BaseWeaponCriticalMultiplier.BaseValue = s_originalSecondarySettings.BaseWeaponCriticalMultiplier;
                    secondary->BaseWeaponCriticalMultiplier.MinMaxRange.X = s_originalSecondarySettings.BaseWeaponCriticalMultiplierMinRange;
                    secondary->BaseWeaponCriticalMultiplier.MinMaxRange.Y = s_originalSecondarySettings.BaseWeaponCriticalMultiplierMaxRange;
                    secondary->BaseRateOfFire.BaseValue = s_originalSecondarySettings.BaseRateOfFire;
                    secondary->BaseRateOfFire.MinMaxRange.X = s_originalSecondarySettings.BaseRateOfFireMinRange;
                    secondary->BaseRateOfFire.MinMaxRange.Y = s_originalSecondarySettings.BaseRateOfFireMaxRange;
                    secondary->BaseCooldown.BaseValue = s_originalSecondarySettings.BaseCooldown;
                    secondary->BaseCooldown.MinMaxRange.X = s_originalSecondarySettings.BaseCooldownMinRange;
                    secondary->BaseCooldown.MinMaxRange.Y = s_originalSecondarySettings.BaseCooldownMaxRange;
                    secondary->BaseRecoil.BaseValue = s_originalSecondarySettings.BaseRecoil;
                    secondary->BaseRecoil.MinMaxRange.X = s_originalSecondarySettings.BaseRecoilMinRange;
                    secondary->BaseRecoil.MinMaxRange.Y = s_originalSecondarySettings.BaseRecoilMaxRange;
                    secondary->BaseRecoilRecovery.BaseValue = s_originalSecondarySettings.BaseRecoilRecovery;
                    secondary->BaseRecoilRecovery.MinMaxRange.X = s_originalSecondarySettings.BaseRecoilRecoveryMinRange;
                    secondary->BaseRecoilRecovery.MinMaxRange.Y = s_originalSecondarySettings.BaseRecoilRecoveryMaxRange;
                    secondary->BaseSpread.BaseValue = s_originalSecondarySettings.BaseSpread;
                    secondary->BaseSpread.MinMaxRange.X = s_originalSecondarySettings.BaseSpreadMinRange;
                    secondary->BaseSpread.MinMaxRange.Y = s_originalSecondarySettings.BaseSpreadMaxRange;
                    secondary->BaseReloadTime.BaseValue = s_originalSecondarySettings.BaseReloadTime;
                    secondary->BaseReloadTime.MinMaxRange.X = s_originalSecondarySettings.BaseReloadTimeMinRange;
                    secondary->BaseReloadTime.MinMaxRange.Y = s_originalSecondarySettings.BaseReloadTimeMaxRange;
                    secondary->ReloadTimeDelta = s_originalSecondarySettings.ReloadTimeDelta;

                    s_cachedEngineRifleScript->SecondaryWeaponModScript->ApplyFireSettings(secondary);
                }
            }

            LOG_INFO("Original weapon settings restored successfully");
        }

        void WeaponManager::ApplyToSecondaryWeapon(SDK::URGWeaponScript* weaponScript, bool restore) {
            // Check if this is an Engine Rifle with secondary weapon mod script
            if (s_isEngineRifle && s_cachedEngineRifleScript && s_cachedEngineRifleScript->SecondaryWeaponModScript) {
                auto baseSettings = weaponScript->GetBaseWeaponSettings();
                if (baseSettings) {
                    if (restore) {
                        LOG_INFO("Restoring settings to secondary weapon");
                    } else {
                        LOG_INFO("Applying modifications to secondary weapon");
                    }
                    s_cachedEngineRifleScript->SecondaryWeaponModScript->ApplyFireSettings(baseSettings);
                }
            }
        }

        void WeaponManager::PrintWeaponInfo(SDK::ARWeapon* weapon) {
            LOG_INFO("Current ammo: %d", weapon->RuntimeWeaponScript->GetAmmoInClip());

            if (weapon->RuntimeWeaponScript->IsA(SDK::UBP_EngineRifle_Script_C::StaticClass())) {
                LOG_INFO("Weapon is a BP_EngineRifle_Script");
                auto engineRifleScript = static_cast<SDK::UBP_EngineRifle_Script_C*>(weapon->RuntimeWeaponScript);
                LOG_INFO("Current heat: %f", engineRifleScript->CurrentHeat);
            } else {
                LOG_INFO("Weapon is NOT a BP_EngineRifle_Script");
            }
        }

    } // namespace Features
} // namespace Cheat
