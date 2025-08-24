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
        OriginalWeaponSettings WeaponManager::s_originalSettings;
        bool WeaponManager::s_originalSettingsSaved = false;
        
        void WeaponManager::Initialize() {
            LOG_INFO("WeaponManager initialized");
        }
        
        void WeaponManager::Update(SDK::ARPlayerPawn* playerPawn) {
            UpdateWeaponCache(playerPawn);
            if (Cheat::Config::Features::EngineRifleHeatManagement) {
                ManageEngineRifleHeat();
            }

            // Handle weapon modifications based on individual flags
            if (s_cachedWeaponScript) {
                // Track previous states for individual restoration
                static bool lastInfiniteAmmo = false;
                static bool lastIncreasedDamage = false;
                static bool lastHighCritMultiplier = false;
                static bool lastFastRateOfFire = false;
                static bool lastNoCooldown = false;
                static bool lastNoRecoil = false;
                static bool lastInstantReload = false;

                bool anyModEnabled = Cheat::Config::Features::InfiniteAmmo ||
                                   Cheat::Config::Features::IncreasedDamage ||
                                   Cheat::Config::Features::HighCritMultiplier ||
                                   Cheat::Config::Features::FastRateOfFire ||
                                   Cheat::Config::Features::NoCooldown ||
                                   Cheat::Config::Features::NoRecoil ||
                                   Cheat::Config::Features::InstantReload;

                // Save original settings if not already saved and any mod is enabled
                if (anyModEnabled && !s_originalSettingsSaved) {
                    SaveOriginalSettings(s_cachedWeaponScript);
                }

                // Check for individual setting changes and apply/restore accordingly
                bool needsUpdate = false;

                if (Cheat::Config::Features::InfiniteAmmo != lastInfiniteAmmo ||
                    Cheat::Config::Features::IncreasedDamage != lastIncreasedDamage ||
                    Cheat::Config::Features::HighCritMultiplier != lastHighCritMultiplier ||
                    Cheat::Config::Features::FastRateOfFire != lastFastRateOfFire ||
                    Cheat::Config::Features::NoCooldown != lastNoCooldown ||
                    Cheat::Config::Features::NoRecoil != lastNoRecoil ||
                    Cheat::Config::Features::InstantReload != lastInstantReload) {
                    needsUpdate = true;
                }

                if (needsUpdate && s_originalSettingsSaved) {
                    ApplyWeaponSettings(s_cachedWeaponScript);
                }

                // Update previous states
                lastInfiniteAmmo = Cheat::Config::Features::InfiniteAmmo;
                lastIncreasedDamage = Cheat::Config::Features::IncreasedDamage;
                lastHighCritMultiplier = Cheat::Config::Features::HighCritMultiplier;
                lastFastRateOfFire = Cheat::Config::Features::FastRateOfFire;
                lastNoCooldown = Cheat::Config::Features::NoCooldown;
                lastNoRecoil = Cheat::Config::Features::NoRecoil;
                lastInstantReload = Cheat::Config::Features::InstantReload;

                // If no mods are enabled and we had saved settings, restore everything
                if (!anyModEnabled && s_originalSettingsSaved) {
                    RestoreOriginalSettings(s_cachedWeaponScript);
                    s_originalSettingsSaved = false;
                }
            }
        }
        
        void WeaponManager::Shutdown() {
            s_cachedCharacter = nullptr;
            s_cachedWeapon = nullptr;
            s_cachedWeaponScript = nullptr;
            s_cachedEngineRifleScript = nullptr;
            s_isEngineRifle = false;
            s_originalSettings = OriginalWeaponSettings();
            s_originalSettingsSaved = false;
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
                s_originalSettings = OriginalWeaponSettings();
                s_originalSettingsSaved = false;

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
        
        void WeaponManager::ApplyWeaponSettings(SDK::URGWeaponScript* weaponScript) {
            auto weaponSettings = weaponScript->GetBaseWeaponSettings();
            if (!weaponSettings) {
                LOG_ERROR("Failed to get weapon settings");
                return;
            }

            if (!s_originalSettings.isValid) {
                LOG_ERROR("No original settings available for restoration");
                return;
            }

            LOG_INFO("Applying/restoring individual weapon modifications...");

            // Apply or restore ammo settings
            if (Cheat::Config::Features::InfiniteAmmo) {
                weaponSettings->BaseAmmoCost.BaseValue = 0;
                LOG_INFO("- Applied: Infinite ammo");
            } else {
                weaponSettings->BaseAmmoCost.BaseValue = s_originalSettings.BaseAmmoCost;
                LOG_INFO("- Restored: Original ammo cost (%d)", s_originalSettings.BaseAmmoCost);
            }

            // Apply or restore damage settings
            if (Cheat::Config::Features::IncreasedDamage) {
                weaponSettings->BaseWeaponDamage.BaseValue = 200.0f;
                weaponSettings->BaseWeaponDamage.MinMaxRange.X = 0.0f;
                weaponSettings->BaseWeaponDamage.MinMaxRange.Y = 1000.0f;
                LOG_INFO("- Applied: Increased damage (200)");
            } else {
                weaponSettings->BaseWeaponDamage.BaseValue = s_originalSettings.BaseWeaponDamage;
                weaponSettings->BaseWeaponDamage.MinMaxRange.X = s_originalSettings.BaseWeaponDamageMinRange;
                weaponSettings->BaseWeaponDamage.MinMaxRange.Y = s_originalSettings.BaseWeaponDamageMaxRange;
                LOG_INFO("- Restored: Original damage (%.1f)", s_originalSettings.BaseWeaponDamage);
            }

            // Apply or restore critical hit settings
            if (Cheat::Config::Features::HighCritMultiplier) {
                weaponSettings->BaseWeaponCriticalMultiplier.BaseValue = 5.0f;
                weaponSettings->BaseWeaponCriticalMultiplier.MinMaxRange.X = 0.0f;
                weaponSettings->BaseWeaponCriticalMultiplier.MinMaxRange.Y = 9999.0f;
                LOG_INFO("- Applied: High critical multiplier (5x)");
            } else {
                weaponSettings->BaseWeaponCriticalMultiplier.BaseValue = s_originalSettings.BaseWeaponCriticalMultiplier;
                weaponSettings->BaseWeaponCriticalMultiplier.MinMaxRange.X = s_originalSettings.BaseWeaponCriticalMultiplierMinRange;
                weaponSettings->BaseWeaponCriticalMultiplier.MinMaxRange.Y = s_originalSettings.BaseWeaponCriticalMultiplierMaxRange;
                LOG_INFO("- Restored: Original critical multiplier (%.1f)", s_originalSettings.BaseWeaponCriticalMultiplier);
            }

            // Apply or restore rate of fire settings
            if (Cheat::Config::Features::FastRateOfFire) {
                weaponSettings->BaseRateOfFire.BaseValue = 20.0f;
                weaponSettings->BaseRateOfFire.MinMaxRange.X = 1.0f;
                weaponSettings->BaseRateOfFire.MinMaxRange.Y = 20.0f;
                LOG_INFO("- Applied: Fast rate of fire (20)");
            } else {
                weaponSettings->BaseRateOfFire.BaseValue = s_originalSettings.BaseRateOfFire;
                weaponSettings->BaseRateOfFire.MinMaxRange.X = s_originalSettings.BaseRateOfFireMinRange;
                weaponSettings->BaseRateOfFire.MinMaxRange.Y = s_originalSettings.BaseRateOfFireMaxRange;
                LOG_INFO("- Restored: Original rate of fire (%.1f)", s_originalSettings.BaseRateOfFire);
            }

            // Apply or restore cooldown settings
            if (Cheat::Config::Features::NoCooldown) {
                weaponSettings->BaseCooldown.BaseValue = 0.0f;
                weaponSettings->BaseCooldown.MinMaxRange.X = 0.0f;
                weaponSettings->BaseCooldown.MinMaxRange.Y = 0.0f;
                LOG_INFO("- Applied: No cooldown");
            } else {
                weaponSettings->BaseCooldown.BaseValue = s_originalSettings.BaseCooldown;
                weaponSettings->BaseCooldown.MinMaxRange.X = s_originalSettings.BaseCooldownMinRange;
                weaponSettings->BaseCooldown.MinMaxRange.Y = s_originalSettings.BaseCooldownMaxRange;
                LOG_INFO("- Restored: Original cooldown (%.1f)", s_originalSettings.BaseCooldown);
            }

            // Apply or restore recoil settings (consolidated: recoil, recoil recovery, and spread)
            if (Cheat::Config::Features::NoRecoil) {
                // No recoil
                weaponSettings->BaseRecoil.BaseValue = 0.0f;
                weaponSettings->BaseRecoil.MinMaxRange.X = 0.0f;
                weaponSettings->BaseRecoil.MinMaxRange.Y = 0.0f;

                // Instant recoil recovery
                weaponSettings->BaseRecoilRecovery.BaseValue = 100.0f;
                weaponSettings->BaseRecoilRecovery.MinMaxRange.X = 0.0f;
                weaponSettings->BaseRecoilRecovery.MinMaxRange.Y = 9999.0f;

                // Perfect accuracy (no spread)
                weaponSettings->BaseSpread.BaseValue = 0.0f;
                weaponSettings->BaseSpread.MinMaxRange.X = 0.0f;
                weaponSettings->BaseSpread.MinMaxRange.Y = 0.0f;

                LOG_INFO("- Applied: No recoil + instant recovery + perfect accuracy");
            } else {
                // Restore original recoil
                weaponSettings->BaseRecoil.BaseValue = s_originalSettings.BaseRecoil;
                weaponSettings->BaseRecoil.MinMaxRange.X = s_originalSettings.BaseRecoilMinRange;
                weaponSettings->BaseRecoil.MinMaxRange.Y = s_originalSettings.BaseRecoilMaxRange;

                // Restore original recoil recovery
                weaponSettings->BaseRecoilRecovery.BaseValue = s_originalSettings.BaseRecoilRecovery;
                weaponSettings->BaseRecoilRecovery.MinMaxRange.X = s_originalSettings.BaseRecoilRecoveryMinRange;
                weaponSettings->BaseRecoilRecovery.MinMaxRange.Y = s_originalSettings.BaseRecoilRecoveryMaxRange;

                // Restore original spread
                weaponSettings->BaseSpread.BaseValue = s_originalSettings.BaseSpread;
                weaponSettings->BaseSpread.MinMaxRange.X = s_originalSettings.BaseSpreadMinRange;
                weaponSettings->BaseSpread.MinMaxRange.Y = s_originalSettings.BaseSpreadMaxRange;

                LOG_INFO("- Restored: Original recoil (%.1f) + recovery (%.1f) + spread (%.1f)",
                    s_originalSettings.BaseRecoil, s_originalSettings.BaseRecoilRecovery, s_originalSettings.BaseSpread);
            }

            // Apply or restore reload settings
            if (Cheat::Config::Features::InstantReload) {
                weaponSettings->BaseReloadTime.BaseValue = 0.01f;
                weaponSettings->BaseReloadTime.MinMaxRange.X = 0.01f;
                weaponSettings->BaseReloadTime.MinMaxRange.Y = 0.01f;
                weaponSettings->ReloadTimeDelta = 0.0f;
                LOG_INFO("- Applied: Instant reload");
            } else {
                weaponSettings->BaseReloadTime.BaseValue = s_originalSettings.BaseReloadTime;
                weaponSettings->BaseReloadTime.MinMaxRange.X = s_originalSettings.BaseReloadTimeMinRange;
                weaponSettings->BaseReloadTime.MinMaxRange.Y = s_originalSettings.BaseReloadTimeMaxRange;
                weaponSettings->ReloadTimeDelta = s_originalSettings.ReloadTimeDelta;
                LOG_INFO("- Restored: Original reload time (%.2f)", s_originalSettings.BaseReloadTime);
            }

            // Apply the settings to primary weapon
            weaponScript->SetBaseWeaponSettings(weaponSettings);

            // Apply to secondary weapon if available
            ApplyToSecondaryWeapon(weaponScript, false);
        }
        
        void WeaponManager::SaveOriginalSettings(SDK::URGWeaponScript* weaponScript) {
            auto weaponSettings = weaponScript->GetBaseWeaponSettings();
            if (!weaponSettings) {
                LOG_ERROR("Failed to get weapon settings for saving originals");
                return;
            }

            LOG_INFO("Saving original weapon settings...");

            // Save ammo settings
            s_originalSettings.BaseAmmoCost = weaponSettings->BaseAmmoCost.BaseValue;

            // Save damage settings
            s_originalSettings.BaseWeaponDamage = weaponSettings->BaseWeaponDamage.BaseValue;
            s_originalSettings.BaseWeaponDamageMinRange = weaponSettings->BaseWeaponDamage.MinMaxRange.X;
            s_originalSettings.BaseWeaponDamageMaxRange = weaponSettings->BaseWeaponDamage.MinMaxRange.Y;

            // Save critical hit settings
            s_originalSettings.BaseWeaponCriticalMultiplier = weaponSettings->BaseWeaponCriticalMultiplier.BaseValue;
            s_originalSettings.BaseWeaponCriticalMultiplierMinRange = weaponSettings->BaseWeaponCriticalMultiplier.MinMaxRange.X;
            s_originalSettings.BaseWeaponCriticalMultiplierMaxRange = weaponSettings->BaseWeaponCriticalMultiplier.MinMaxRange.Y;

            // Save rate of fire settings
            s_originalSettings.BaseRateOfFire = weaponSettings->BaseRateOfFire.BaseValue;
            s_originalSettings.BaseRateOfFireMinRange = weaponSettings->BaseRateOfFire.MinMaxRange.X;
            s_originalSettings.BaseRateOfFireMaxRange = weaponSettings->BaseRateOfFire.MinMaxRange.Y;

            // Save cooldown settings
            s_originalSettings.BaseCooldown = weaponSettings->BaseCooldown.BaseValue;
            s_originalSettings.BaseCooldownMinRange = weaponSettings->BaseCooldown.MinMaxRange.X;
            s_originalSettings.BaseCooldownMaxRange = weaponSettings->BaseCooldown.MinMaxRange.Y;

            // Save recoil settings
            s_originalSettings.BaseRecoil = weaponSettings->BaseRecoil.BaseValue;
            s_originalSettings.BaseRecoilMinRange = weaponSettings->BaseRecoil.MinMaxRange.X;
            s_originalSettings.BaseRecoilMaxRange = weaponSettings->BaseRecoil.MinMaxRange.Y;

            // Save recoil recovery settings
            s_originalSettings.BaseRecoilRecovery = weaponSettings->BaseRecoilRecovery.BaseValue;
            s_originalSettings.BaseRecoilRecoveryMinRange = weaponSettings->BaseRecoilRecovery.MinMaxRange.X;
            s_originalSettings.BaseRecoilRecoveryMaxRange = weaponSettings->BaseRecoilRecovery.MinMaxRange.Y;

            // Save spread settings
            s_originalSettings.BaseSpread = weaponSettings->BaseSpread.BaseValue;
            s_originalSettings.BaseSpreadMinRange = weaponSettings->BaseSpread.MinMaxRange.X;
            s_originalSettings.BaseSpreadMaxRange = weaponSettings->BaseSpread.MinMaxRange.Y;

            // Save reload settings
            s_originalSettings.BaseReloadTime = weaponSettings->BaseReloadTime.BaseValue;
            s_originalSettings.BaseReloadTimeMinRange = weaponSettings->BaseReloadTime.MinMaxRange.X;
            s_originalSettings.BaseReloadTimeMaxRange = weaponSettings->BaseReloadTime.MinMaxRange.Y;
            s_originalSettings.ReloadTimeDelta = weaponSettings->ReloadTimeDelta;

            s_originalSettings.isValid = true;
            s_originalSettingsSaved = true;

            LOG_INFO("Original weapon settings saved successfully");
        }

        void WeaponManager::RestoreOriginalSettings(SDK::URGWeaponScript* weaponScript) {
            if (!s_originalSettings.isValid) {
                LOG_ERROR("No valid original settings to restore");
                return;
            }

            auto weaponSettings = weaponScript->GetBaseWeaponSettings();
            if (!weaponSettings) {
                LOG_ERROR("Failed to get weapon settings for restoration");
                return;
            }

            LOG_INFO("Restoring original weapon settings...");

            // Restore ammo settings
            weaponSettings->BaseAmmoCost.BaseValue = s_originalSettings.BaseAmmoCost;

            // Restore damage settings
            weaponSettings->BaseWeaponDamage.BaseValue = s_originalSettings.BaseWeaponDamage;
            weaponSettings->BaseWeaponDamage.MinMaxRange.X = s_originalSettings.BaseWeaponDamageMinRange;
            weaponSettings->BaseWeaponDamage.MinMaxRange.Y = s_originalSettings.BaseWeaponDamageMaxRange;

            // Restore critical hit settings
            weaponSettings->BaseWeaponCriticalMultiplier.BaseValue = s_originalSettings.BaseWeaponCriticalMultiplier;
            weaponSettings->BaseWeaponCriticalMultiplier.MinMaxRange.X = s_originalSettings.BaseWeaponCriticalMultiplierMinRange;
            weaponSettings->BaseWeaponCriticalMultiplier.MinMaxRange.Y = s_originalSettings.BaseWeaponCriticalMultiplierMaxRange;

            // Restore rate of fire settings
            weaponSettings->BaseRateOfFire.BaseValue = s_originalSettings.BaseRateOfFire;
            weaponSettings->BaseRateOfFire.MinMaxRange.X = s_originalSettings.BaseRateOfFireMinRange;
            weaponSettings->BaseRateOfFire.MinMaxRange.Y = s_originalSettings.BaseRateOfFireMaxRange;

            // Restore cooldown settings
            weaponSettings->BaseCooldown.BaseValue = s_originalSettings.BaseCooldown;
            weaponSettings->BaseCooldown.MinMaxRange.X = s_originalSettings.BaseCooldownMinRange;
            weaponSettings->BaseCooldown.MinMaxRange.Y = s_originalSettings.BaseCooldownMaxRange;

            // Restore recoil settings
            weaponSettings->BaseRecoil.BaseValue = s_originalSettings.BaseRecoil;
            weaponSettings->BaseRecoil.MinMaxRange.X = s_originalSettings.BaseRecoilMinRange;
            weaponSettings->BaseRecoil.MinMaxRange.Y = s_originalSettings.BaseRecoilMaxRange;

            // Restore recoil recovery settings
            weaponSettings->BaseRecoilRecovery.BaseValue = s_originalSettings.BaseRecoilRecovery;
            weaponSettings->BaseRecoilRecovery.MinMaxRange.X = s_originalSettings.BaseRecoilRecoveryMinRange;
            weaponSettings->BaseRecoilRecovery.MinMaxRange.Y = s_originalSettings.BaseRecoilRecoveryMaxRange;

            // Restore spread settings
            weaponSettings->BaseSpread.BaseValue = s_originalSettings.BaseSpread;
            weaponSettings->BaseSpread.MinMaxRange.X = s_originalSettings.BaseSpreadMinRange;
            weaponSettings->BaseSpread.MinMaxRange.Y = s_originalSettings.BaseSpreadMaxRange;

            // Restore reload settings
            weaponSettings->BaseReloadTime.BaseValue = s_originalSettings.BaseReloadTime;
            weaponSettings->BaseReloadTime.MinMaxRange.X = s_originalSettings.BaseReloadTimeMinRange;
            weaponSettings->BaseReloadTime.MinMaxRange.Y = s_originalSettings.BaseReloadTimeMaxRange;
            weaponSettings->ReloadTimeDelta = s_originalSettings.ReloadTimeDelta;

            // Apply the restored settings to primary weapon
            weaponScript->SetBaseWeaponSettings(weaponSettings);

            // Apply to secondary weapon if available
            ApplyToSecondaryWeapon(weaponScript, true);

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
