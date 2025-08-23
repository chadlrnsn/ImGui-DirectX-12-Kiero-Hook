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
        
        void WeaponManager::Initialize() {
            LOG_INFO("WeaponManager initialized");
        }
        
        void WeaponManager::Update(SDK::ARPlayerPawn* playerPawn) {
            UpdateWeaponCache(playerPawn);
            if (Cheat::Config::Features::EngineRifleHeatManagement) {
                ManageEngineRifleHeat();
            }
        }
        
        void WeaponManager::Shutdown() {
            s_cachedCharacter = nullptr;
            s_cachedWeapon = nullptr;
            s_cachedWeaponScript = nullptr;
            s_cachedEngineRifleScript = nullptr;
            s_isEngineRifle = false;
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
            
            LOG_INFO("Applying weapon modifications...");
            
            // Infinite ammo (no ammo cost)
            weaponSettings->BaseAmmoCost.BaseValue = 0;
            LOG_INFO("- Set ammo cost to 0 (infinite ammo)");
            
            // Massively increased damage
            weaponSettings->BaseWeaponDamage.BaseValue = 200.0f;
            weaponSettings->BaseWeaponDamage.MinMaxRange.X = 0.0f;
            weaponSettings->BaseWeaponDamage.MinMaxRange.Y = 1000.0f;
            LOG_INFO("- Set weapon damage to 200");
            
            // Extremely high critical hit multiplier
            weaponSettings->BaseWeaponCriticalMultiplier.BaseValue = 5.0f;
            weaponSettings->BaseWeaponCriticalMultiplier.MinMaxRange.X = 0.0f;
            weaponSettings->BaseWeaponCriticalMultiplier.MinMaxRange.Y = 9999.0f;
            LOG_INFO("- Set critical multiplier to 5x");
            
            // Super fast rate of fire
            weaponSettings->BaseRateOfFire.BaseValue = 20.0f;
            weaponSettings->BaseRateOfFire.MinMaxRange.X = 1.0f;
            weaponSettings->BaseRateOfFire.MinMaxRange.Y = 20.0f;
            LOG_INFO("- Set rate of fire to 20");
            
            // No cooldown
            weaponSettings->BaseCooldown.BaseValue = 0.0f;
            weaponSettings->BaseCooldown.MinMaxRange.X = 0.0f;
            weaponSettings->BaseCooldown.MinMaxRange.Y = 0.0f;
            LOG_INFO("- Set cooldown to 0");
            
            // No recoil
            weaponSettings->BaseRecoil.BaseValue = 0.0f;
            weaponSettings->BaseRecoil.MinMaxRange.X = 0.0f;
            weaponSettings->BaseRecoil.MinMaxRange.Y = 0.0f;
            LOG_INFO("- Set recoil to 0");
            
            // Instant recoil recovery
            weaponSettings->BaseRecoilRecovery.BaseValue = 100.0f;
            weaponSettings->BaseRecoilRecovery.MinMaxRange.X = 0.0f;
            weaponSettings->BaseRecoilRecovery.MinMaxRange.Y = 9999.0f;
            LOG_INFO("- Set recoil recovery to 100");
            
            // No spread (perfect accuracy)
            weaponSettings->BaseSpread.BaseValue = 0.0f;
            weaponSettings->BaseSpread.MinMaxRange.X = 0.0f;
            weaponSettings->BaseSpread.MinMaxRange.Y = 0.0f;
            LOG_INFO("- Set spread to 0 (perfect accuracy)");
            
            // Instant reload
            weaponSettings->BaseReloadTime.BaseValue = 0.01f;
            weaponSettings->BaseReloadTime.MinMaxRange.X = 0.01f;
            weaponSettings->BaseReloadTime.MinMaxRange.Y = 0.01f;
            weaponSettings->ReloadTimeDelta = 0.0f;
            LOG_INFO("- Set reload time to 0.01 seconds");
            
            // Apply the modified settings
            weaponScript->SetBaseWeaponSettings(weaponSettings);
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
