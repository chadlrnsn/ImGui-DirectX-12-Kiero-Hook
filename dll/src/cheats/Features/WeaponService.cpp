#include "WeaponService.h"
#include "../Core/Config.h"
#include "../Utils/Console.h"
#include "../Services/GameServices.h"
#include <dev/logger.h>

namespace Cheat {
namespace Features {

// Keep implementation identical to WeaponManager.cpp, only class/filename names updated

SDK::ARPlayerPawn* WeaponService::s_cachedPlayerPawn = nullptr;
SDK::ARWeapon* WeaponService::s_cachedWeapon = nullptr;
SDK::URGWeaponScript* WeaponService::s_cachedWeaponScript = nullptr;
SDK::UBP_EngineRifle_Script_C* WeaponService::s_cachedEngineRifleScript = nullptr;
bool WeaponService::s_isEngineRifle = false;
OriginalWeaponSettings WeaponService::s_originalPrimarySettings;
OriginalWeaponSettings WeaponService::s_originalSecondarySettings;
bool WeaponService::s_primaryOriginalSaved = false;
bool WeaponService::s_secondaryOriginalSaved = false;

void WeaponService::Initialize() { LOG_INFO("WeaponService initialized"); }

void WeaponService::OnWeaponSettingsChanged() {
    if (!s_cachedWeaponScript) return;
    const bool anyEnabled = AnyModsEnabled();
    if (anyEnabled && (!s_primaryOriginalSaved || !s_secondaryOriginalSaved)) {
        // Save originals for both primary and secondary once
        SaveOriginalSettings(s_cachedWeaponScript);
        s_primaryOriginalSaved = true;
        s_secondaryOriginalSaved = true;
    }
    if (anyEnabled) {
        ApplyWeaponSettings(s_cachedWeaponScript);
    } else {
        RestoreOriginalSettings(s_cachedWeaponScript);
        s_primaryOriginalSaved = false;
        s_secondaryOriginalSaved = false;
    }
}

bool WeaponService::AnyModsEnabled() {
    using namespace Cheat::Config::Features;
    return InfiniteAmmo || IncreasedDamage || HighCritMultiplier || NoCooldown || NoRecoil || InstantReload || RateOfFireOverride;
}

void WeaponService::Update() {
    auto pawn = Cheat::Services::GameServices::GetRPlayerPawn();
    UpdateWeaponCache(pawn);
    if (Cheat::Config::Features::NoHeatBuildup) {
        ManageEngineRifleHeat();
    }
}

void WeaponService::Shutdown() {
    s_cachedPlayerPawn = nullptr;
    s_cachedWeapon = nullptr;
    s_cachedWeaponScript = nullptr;
    s_cachedEngineRifleScript = nullptr;
    s_isEngineRifle = false;
    s_originalPrimarySettings = OriginalWeaponSettings();
    s_originalSecondarySettings = OriginalWeaponSettings();
    s_primaryOriginalSaved = false;
    s_secondaryOriginalSaved = false;
    LOG_INFO("WeaponService shutdown");
}

void WeaponService::ApplyModifications(SDK::UWorld* world) {
    auto playerController = world->OwningGameInstance->LocalPlayers[0]->PlayerController;
    if (!playerController) { LOG_ERROR("PlayerController not found"); return; }
    auto character = static_cast<SDK::ARPlayerPawn*>(playerController->K2_GetPawn());
    if (!character) { LOG_ERROR("Player Pawn not spawned yet"); return; }
    auto weapon = character->GetEquippedWeapon();
    if (!weapon) { LOG_ERROR("Player weapon not found"); return; }
    PrintWeaponInfo(weapon);
    auto weaponScript = weapon->RuntimeWeaponScript;
    if (!weaponScript) { LOG_ERROR("Weapon Runtime Script not found"); return; }
    ApplyWeaponSettings(weaponScript);
    LOG_INFO("Weapon modifications applied successfully!");
}

void WeaponService::UpdateWeaponCache(SDK::ARPlayerPawn* character) {
    if (!character) {
        s_cachedPlayerPawn = nullptr;
        s_cachedWeapon = nullptr;
        s_cachedWeaponScript = nullptr;
        s_cachedEngineRifleScript = nullptr;
        s_isEngineRifle = false;
        return;
    }
    s_cachedPlayerPawn = character;
    auto weapon = character->GetEquippedWeapon();

    if (weapon != s_cachedWeapon) {
        // Invalidate cache when the equipped weapon actor changes
        s_cachedWeapon = weapon;
        s_cachedWeaponScript = nullptr;
        s_cachedEngineRifleScript = nullptr;
        s_isEngineRifle = false;

        // Reset saved originals for previous weapon
        s_originalPrimarySettings = OriginalWeaponSettings();
        s_originalSecondarySettings = OriginalWeaponSettings();
        s_primaryOriginalSaved = false;
        s_secondaryOriginalSaved = false;

        if (weapon && weapon->RuntimeWeaponScript) {
            s_cachedWeaponScript = weapon->RuntimeWeaponScript;

            if (s_cachedWeaponScript->IsA(SDK::UBP_EngineRifle_Script_C::StaticClass())) {
                s_cachedEngineRifleScript = static_cast<SDK::UBP_EngineRifle_Script_C*>(s_cachedWeaponScript);
                s_isEngineRifle = true;
            }

            // Save originals for the new weapon's primary and secondary immediately
            SaveOriginalSettings(s_cachedWeaponScript);
            s_primaryOriginalSaved = true;
            s_secondaryOriginalSaved = true;

            // If any mods are enabled, apply them after saving new originals
            if (AnyModsEnabled()) {
                ApplyWeaponSettings(s_cachedWeaponScript);
            }
        }
    }
}

void WeaponService::ManageEngineRifleHeat() {
    if (s_isEngineRifle && s_cachedEngineRifleScript) {
        // Force set heat to 0 to prevent overheating
        s_cachedEngineRifleScript->ForceSetHeat(0.0);
    }
}


static float Clampf(float v, float minv, float maxv) { if (v < minv) return minv; if (v > maxv) return maxv; return v; }

void WeaponService::ApplyWeaponSettings(SDK::URGWeaponScript* weaponScript) {
    if (!weaponScript) return;

    // Ensure originals are saved once when enabling
    if ((!s_primaryOriginalSaved || !s_secondaryOriginalSaved) && AnyModsEnabled()) {
        SaveOriginalSettings(weaponScript);
        s_primaryOriginalSaved = true;
        s_secondaryOriginalSaved = true;
    }

    // Apply for primary and secondary independently
    ApplyWeaponSettingsFor(weaponScript, true);
    ApplyWeaponSettingsFor(weaponScript, false);
}


void WeaponService::ApplyWeaponSettingsFor(SDK::URGWeaponScript* weaponScript, bool isPrimary) {
    using namespace Cheat::Config::Features;
    SDK::URBaseWeaponSettings* ws = isPrimary ? GetPrimarySettings(weaponScript) : GetSecondarySettings(weaponScript);
    if (!ws) return;

    OriginalWeaponSettings& orig = isPrimary ? s_originalPrimarySettings : s_originalSecondarySettings;

    // Ammo
    ws->BaseAmmoCost.BaseValue = InfiniteAmmo ? 0 : orig.BaseAmmoCost;

    // Damage
    if (IncreasedDamage) {
        const float minV = orig.BaseWeaponDamageMinRange;
        const float maxV = 9999.0f;
        ws->BaseWeaponDamage.BaseValue = Clampf(orig.BaseWeaponDamage * DamageMultiplier, minV, maxV);
    } else {
        ws->BaseWeaponDamage.BaseValue = orig.BaseWeaponDamage;
    }
    ws->BaseWeaponDamage.MinMaxRange.X = orig.BaseWeaponDamageMinRange;
    ws->BaseWeaponDamage.MinMaxRange.Y = IncreasedDamage ? 9999.0f : orig.BaseWeaponDamageMaxRange;

    // Crit
    if (HighCritMultiplier) {
        const float minV = orig.BaseWeaponCriticalMultiplierMinRange;
        const float maxV = orig.BaseWeaponCriticalMultiplierMaxRange;
        ws->BaseWeaponCriticalMultiplier.BaseValue = Clampf(orig.BaseWeaponCriticalMultiplier * CritMultiplier, minV, maxV);
    } else {
        ws->BaseWeaponCriticalMultiplier.BaseValue = orig.BaseWeaponCriticalMultiplier;
    }
    ws->BaseWeaponCriticalMultiplier.MinMaxRange.X = orig.BaseWeaponCriticalMultiplierMinRange;
    ws->BaseWeaponCriticalMultiplier.MinMaxRange.Y = orig.BaseWeaponCriticalMultiplierMaxRange;

    // Rate of Fire
    if (RateOfFireOverride) {
        float minV = orig.BaseRateOfFireMinRange;
        float maxV = 100.0f;
        float v = RateOfFireValue;
        if (v <= 0.0f) v = orig.BaseRateOfFire;
        ws->BaseRateOfFire.BaseValue = Clampf(v, minV, maxV);
        ws->BaseRateOfFire.MinMaxRange.X = minV;
        ws->BaseRateOfFire.MinMaxRange.Y = 100.0f;
    } else {
        ws->BaseRateOfFire.BaseValue = orig.BaseRateOfFire;
        ws->BaseRateOfFire.MinMaxRange.X = orig.BaseRateOfFireMinRange;
        ws->BaseRateOfFire.MinMaxRange.Y = orig.BaseRateOfFireMaxRange;
    }

    // Cooldown
    if (NoCooldown) {
        ws->BaseCooldown.BaseValue = 0.0f;
        ws->BaseCooldown.MinMaxRange.X = 0.0f;
        ws->BaseCooldown.MinMaxRange.Y = 0.0f;
    } else {
        ws->BaseCooldown.BaseValue = orig.BaseCooldown;
        ws->BaseCooldown.MinMaxRange.X = orig.BaseCooldownMinRange;
        ws->BaseCooldown.MinMaxRange.Y = orig.BaseCooldownMaxRange;
    }

    // Recoil/Recovery/Spread
    if (NoRecoil) {
        ws->BaseRecoil.BaseValue = 0.0f;
        ws->BaseRecoil.MinMaxRange.X = 0.0f;
        ws->BaseRecoil.MinMaxRange.Y = 0.0f;
        ws->BaseRecoilRecovery.BaseValue = 100.0f;
        ws->BaseRecoilRecovery.MinMaxRange.X = 0.0f;
        ws->BaseRecoilRecovery.MinMaxRange.Y = 9999.0f;
        ws->BaseSpread.BaseValue = 0.0f;
        ws->BaseSpread.MinMaxRange.X = 0.0f;
        ws->BaseSpread.MinMaxRange.Y = 0.0f;
    } else {
        ws->BaseRecoil.BaseValue = orig.BaseRecoil;
        ws->BaseRecoil.MinMaxRange.X = orig.BaseRecoilMinRange;
        ws->BaseRecoil.MinMaxRange.Y = orig.BaseRecoilMaxRange;
        ws->BaseRecoilRecovery.BaseValue = orig.BaseRecoilRecovery;
        ws->BaseRecoilRecovery.MinMaxRange.X = orig.BaseRecoilRecoveryMinRange;
        ws->BaseRecoilRecovery.MinMaxRange.Y = orig.BaseRecoilRecoveryMaxRange;
        ws->BaseSpread.BaseValue = orig.BaseSpread;
        ws->BaseSpread.MinMaxRange.X = orig.BaseSpreadMinRange;
        ws->BaseSpread.MinMaxRange.Y = orig.BaseSpreadMaxRange;
    }

    // Reload
    if (InstantReload) {
        ws->BaseReloadTime.BaseValue = 0.1f;
        ws->BaseReloadTime.MinMaxRange.X = 0.1f;
        ws->BaseReloadTime.MinMaxRange.Y = 1.6f;
        ws->ReloadTimeDelta = 0.1f;
    } else {
        ws->BaseReloadTime.BaseValue = orig.BaseReloadTime;
        ws->BaseReloadTime.MinMaxRange.X = orig.BaseReloadTimeMinRange;
        ws->BaseReloadTime.MinMaxRange.Y = orig.BaseReloadTimeMaxRange;
        ws->ReloadTimeDelta = orig.ReloadTimeDelta;
    }

    // Push to the correct mod script for runtime effect
    if (isPrimary && weaponScript->PrimaryWeaponModScript) {
        weaponScript->PrimaryWeaponModScript->ApplyFireSettings(ws);
    } else if (!isPrimary && weaponScript->SecondaryWeaponModScript) {
        weaponScript->SecondaryWeaponModScript->ApplyFireSettings(ws);
    }
}

void WeaponService::LogAllWeaponStats() {
    if (!s_cachedWeaponScript) {
        LOG_INFO("=== WEAPON STATS DEBUG ===");
        LOG_INFO("No cached weapon script available");
        return;
    }
    LOG_INFO("=== WEAPON STATS DEBUG ===");

    auto primary = GetPrimarySettings(s_cachedWeaponScript);
    if (primary) {
        LOG_INFO("--- PRIMARY WEAPON ---");
        LOG_INFO("Ammo Cost: %d", primary->BaseAmmoCost.BaseValue);
        LOG_INFO("Damage: %.2f (Range: %.2f - %.2f)", primary->BaseWeaponDamage.BaseValue, primary->BaseWeaponDamage.MinMaxRange.X, primary->BaseWeaponDamage.MinMaxRange.Y);
        LOG_INFO("Crit Multiplier: %.2f (Range: %.2f - %.2f)", primary->BaseWeaponCriticalMultiplier.BaseValue, primary->BaseWeaponCriticalMultiplier.MinMaxRange.X, primary->BaseWeaponCriticalMultiplier.MinMaxRange.Y);
        LOG_INFO("Rate of Fire: %.2f (Range: %.2f - %.2f)", primary->BaseRateOfFire.BaseValue, primary->BaseRateOfFire.MinMaxRange.X, primary->BaseRateOfFire.MinMaxRange.Y);
        LOG_INFO("Cooldown: %.2f (Range: %.2f - %.2f)", primary->BaseCooldown.BaseValue, primary->BaseCooldown.MinMaxRange.X, primary->BaseCooldown.MinMaxRange.Y);
        LOG_INFO("Recoil: %.2f (Range: %.2f - %.2f)", primary->BaseRecoil.BaseValue, primary->BaseRecoil.MinMaxRange.X, primary->BaseRecoil.MinMaxRange.Y);
        LOG_INFO("Recoil Recovery: %.2f (Range: %.2f - %.2f)", primary->BaseRecoilRecovery.BaseValue, primary->BaseRecoilRecovery.MinMaxRange.X, primary->BaseRecoilRecovery.MinMaxRange.Y);
        LOG_INFO("Spread: %.2f (Range: %.2f - %.2f)", primary->BaseSpread.BaseValue, primary->BaseSpread.MinMaxRange.X, primary->BaseSpread.MinMaxRange.Y);
        LOG_INFO("Reload Time: %.2f (Range: %.2f - %.2f), Delta: %.2f", primary->BaseReloadTime.BaseValue, primary->BaseReloadTime.MinMaxRange.X, primary->BaseReloadTime.MinMaxRange.Y, primary->ReloadTimeDelta);
    } else {
        LOG_INFO("--- PRIMARY WEAPON ---");
        LOG_INFO("Failed to get primary weapon settings");
    }

    if (auto secondary = GetSecondarySettings(s_cachedWeaponScript)) {
        LOG_INFO("--- SECONDARY WEAPON ---");
        LOG_INFO("Ammo Cost: %d", secondary->BaseAmmoCost.BaseValue);
        LOG_INFO("Damage: %.2f (Range: %.2f - %.2f)", secondary->BaseWeaponDamage.BaseValue, secondary->BaseWeaponDamage.MinMaxRange.X, secondary->BaseWeaponDamage.MinMaxRange.Y);
        LOG_INFO("Crit Multiplier: %.2f (Range: %.2f - %.2f)", secondary->BaseWeaponCriticalMultiplier.BaseValue, secondary->BaseWeaponCriticalMultiplier.MinMaxRange.X, secondary->BaseWeaponCriticalMultiplier.MinMaxRange.Y);
        LOG_INFO("Rate of Fire: %.2f (Range: %.2f - %.2f)", secondary->BaseRateOfFire.BaseValue, secondary->BaseRateOfFire.MinMaxRange.X, secondary->BaseRateOfFire.MinMaxRange.Y);
        LOG_INFO("Cooldown: %.2f (Range: %.2f - %.2f)", secondary->BaseCooldown.BaseValue, secondary->BaseCooldown.MinMaxRange.X, secondary->BaseCooldown.MinMaxRange.Y);
        LOG_INFO("Recoil: %.2f (Range: %.2f - %.2f)", secondary->BaseRecoil.BaseValue, secondary->BaseRecoil.MinMaxRange.X, secondary->BaseRecoil.MinMaxRange.Y);
        LOG_INFO("Recoil Recovery: %.2f (Range: %.2f - %.2f)", secondary->BaseRecoilRecovery.BaseValue, secondary->BaseRecoilRecovery.MinMaxRange.X, secondary->BaseRecoilRecovery.MinMaxRange.Y);
        LOG_INFO("Spread: %.2f (Range: %.2f - %.2f)", secondary->BaseSpread.BaseValue, secondary->BaseSpread.MinMaxRange.X, secondary->BaseSpread.MinMaxRange.Y);
        LOG_INFO("Reload Time: %.2f (Range: %.2f - %.2f), Delta: %.2f", secondary->BaseReloadTime.BaseValue, secondary->BaseReloadTime.MinMaxRange.X, secondary->BaseReloadTime.MinMaxRange.Y, secondary->ReloadTimeDelta);
    } else {
        LOG_INFO("--- SECONDARY WEAPON ---");
        LOG_INFO("No secondary weapon mod script/stats available");
    }

    if (s_primaryOriginalSaved) {
        LOG_INFO("--- ORIGINAL PRIMARY SETTINGS (SAVED) ---");
        LOG_INFO("Ammo Cost: %d", s_originalPrimarySettings.BaseAmmoCost);
        LOG_INFO("Damage: %.2f (Range: %.2f - %.2f)", s_originalPrimarySettings.BaseWeaponDamage, s_originalPrimarySettings.BaseWeaponDamageMinRange, s_originalPrimarySettings.BaseWeaponDamageMaxRange);
        LOG_INFO("Crit Multiplier: %.2f (Range: %.2f - %.2f)", s_originalPrimarySettings.BaseWeaponCriticalMultiplier, s_originalPrimarySettings.BaseWeaponCriticalMultiplierMinRange, s_originalPrimarySettings.BaseWeaponCriticalMultiplierMaxRange);
        LOG_INFO("Rate of Fire: %.2f (Range: %.2f - %.2f)", s_originalPrimarySettings.BaseRateOfFire, s_originalPrimarySettings.BaseRateOfFireMinRange, s_originalPrimarySettings.BaseRateOfFireMaxRange);
        LOG_INFO("Cooldown: %.2f (Range: %.2f - %.2f)", s_originalPrimarySettings.BaseCooldown, s_originalPrimarySettings.BaseCooldownMinRange, s_originalPrimarySettings.BaseCooldownMaxRange);
        LOG_INFO("Recoil: %.2f (Range: %.2f - %.2f)", s_originalPrimarySettings.BaseRecoil, s_originalPrimarySettings.BaseRecoilMinRange, s_originalPrimarySettings.BaseRecoilMaxRange);
        LOG_INFO("Recoil Recovery: %.2f (Range: %.2f - %.2f)", s_originalPrimarySettings.BaseRecoilRecovery, s_originalPrimarySettings.BaseRecoilRecoveryMinRange, s_originalPrimarySettings.BaseRecoilRecoveryMaxRange);
        LOG_INFO("Spread: %.2f (Range: %.2f - %.2f)", s_originalPrimarySettings.BaseSpread, s_originalPrimarySettings.BaseSpreadMinRange, s_originalPrimarySettings.BaseSpreadMaxRange);
        LOG_INFO("Reload Time: %.2f (Range: %.2f - %.2f), Delta: %.2f", s_originalPrimarySettings.BaseReloadTime, s_originalPrimarySettings.BaseReloadTimeMinRange, s_originalPrimarySettings.BaseReloadTimeMaxRange, s_originalPrimarySettings.ReloadTimeDelta);
    } else {
        LOG_INFO("--- ORIGINAL PRIMARY SETTINGS ---");
        LOG_INFO("Not saved yet");
    }
    LOG_INFO("=== END WEAPON STATS DEBUG ===");
}

void WeaponService::SaveOriginalSettings(SDK::URGWeaponScript* weaponScript) {
    // Save primary
    if (auto ws = GetPrimarySettings(weaponScript)) {
        LOG_INFO("Saving original PRIMARY weapon settings...");
        SaveOriginalSettingsFor(ws, s_originalPrimarySettings);
    }
    // Save secondary if present
    if (auto ss = GetSecondarySettings(weaponScript)) {
        LOG_INFO("Saving original SECONDARY weapon settings...");
        SaveOriginalSettingsFor(ss, s_originalSecondarySettings);
    }
}

void WeaponService::RestoreOriginalSettings(SDK::URGWeaponScript* weaponScript) {
    if (!s_originalPrimarySettings.isValid) { LOG_ERROR("No valid original PRIMARY settings to restore"); return; }

    LOG_INFO("Restoring original weapon settings (primary/secondary)...");
    RestoreSettingsFor(weaponScript, true);
    RestoreSettingsFor(weaponScript, false);
    LOG_INFO("Original weapon settings restored successfully");
}


void WeaponService::RestoreSettingsFor(SDK::URGWeaponScript* weaponScript, bool isPrimary) {
    SDK::URBaseWeaponSettings* ws = isPrimary ? GetPrimarySettings(weaponScript) : GetSecondarySettings(weaponScript);
    if (!ws) return;
    OriginalWeaponSettings& orig = isPrimary ? s_originalPrimarySettings : s_originalSecondarySettings;
    if (!orig.isValid) return;

    ws->BaseAmmoCost.BaseValue = orig.BaseAmmoCost;
    ws->BaseWeaponDamage.BaseValue = orig.BaseWeaponDamage;
    ws->BaseWeaponDamage.MinMaxRange.X = orig.BaseWeaponDamageMinRange;
    ws->BaseWeaponDamage.MinMaxRange.Y = orig.BaseWeaponDamageMaxRange;
    ws->BaseWeaponCriticalMultiplier.BaseValue = orig.BaseWeaponCriticalMultiplier;
    ws->BaseWeaponCriticalMultiplier.MinMaxRange.X = orig.BaseWeaponCriticalMultiplierMinRange;
    ws->BaseWeaponCriticalMultiplier.MinMaxRange.Y = orig.BaseWeaponCriticalMultiplierMaxRange;
    ws->BaseRateOfFire.BaseValue = orig.BaseRateOfFire;
    ws->BaseRateOfFire.MinMaxRange.X = orig.BaseRateOfFireMinRange;
    ws->BaseRateOfFire.MinMaxRange.Y = orig.BaseRateOfFireMaxRange;
    ws->BaseCooldown.BaseValue = orig.BaseCooldown;
    ws->BaseCooldown.MinMaxRange.X = orig.BaseCooldownMinRange;
    ws->BaseCooldown.MinMaxRange.Y = orig.BaseCooldownMaxRange;
    ws->BaseRecoil.BaseValue = orig.BaseRecoil;
    ws->BaseRecoil.MinMaxRange.X = orig.BaseRecoilMinRange;
    ws->BaseRecoil.MinMaxRange.Y = orig.BaseRecoilMaxRange;
    ws->BaseRecoilRecovery.BaseValue = orig.BaseRecoilRecovery;
    ws->BaseRecoilRecovery.MinMaxRange.X = orig.BaseRecoilRecoveryMinRange;
    ws->BaseRecoilRecovery.MinMaxRange.Y = orig.BaseRecoilRecoveryMaxRange;
    ws->BaseSpread.BaseValue = orig.BaseSpread;
    ws->BaseSpread.MinMaxRange.X = orig.BaseSpreadMinRange;
    ws->BaseSpread.MinMaxRange.Y = orig.BaseSpreadMaxRange;
    ws->BaseReloadTime.BaseValue = orig.BaseReloadTime;
    ws->BaseReloadTime.MinMaxRange.X = orig.BaseReloadTimeMinRange;
    ws->BaseReloadTime.MinMaxRange.Y = orig.BaseReloadTimeMaxRange;
    ws->ReloadTimeDelta = orig.ReloadTimeDelta;

    // Push to the correct mod script for runtime effect
    if (isPrimary && weaponScript->PrimaryWeaponModScript) {
        weaponScript->PrimaryWeaponModScript->ApplyFireSettings(ws);
    } else if (!isPrimary && weaponScript->SecondaryWeaponModScript) {
        weaponScript->SecondaryWeaponModScript->ApplyFireSettings(ws);
    }
}


void WeaponService::PrintWeaponInfo(SDK::ARWeapon* weapon) {
    LOG_INFO("Current ammo: %d", weapon->RuntimeWeaponScript->GetAmmoInClip());
    if (weapon->RuntimeWeaponScript->IsA(SDK::UBP_EngineRifle_Script_C::StaticClass())) {
        LOG_INFO("Weapon is a BP_EngineRifle_Script");
        auto engineRifleScript = static_cast<SDK::UBP_EngineRifle_Script_C*>(weapon->RuntimeWeaponScript);
        LOG_INFO("Current heat: %f", engineRifleScript->CurrentHeat);
    } else {
        LOG_INFO("Weapon is NOT a BP_EngineRifle_Script");
    }
}

// Helpers
SDK::URBaseWeaponSettings* WeaponService::GetPrimarySettings(SDK::URGWeaponScript* ws) {
    if (!ws || !ws->PrimaryWeaponModScript) return nullptr;
    return ws->PrimaryWeaponModScript->WeaponModStats;
}

SDK::URBaseWeaponSettings* WeaponService::GetSecondarySettings(SDK::URGWeaponScript* ws) {
    if (!ws || !ws->SecondaryWeaponModScript) return nullptr;
    return ws->SecondaryWeaponModScript->WeaponModStats;
}

void WeaponService::SaveOriginalSettingsFor(SDK::URBaseWeaponSettings* ws, OriginalWeaponSettings& out) {
    if (!ws) return;
    out.BaseAmmoCost = ws->BaseAmmoCost.BaseValue;
    out.BaseWeaponDamage = ws->BaseWeaponDamage.BaseValue;
    out.BaseWeaponDamageMinRange = ws->BaseWeaponDamage.MinMaxRange.X;
    out.BaseWeaponDamageMaxRange = ws->BaseWeaponDamage.MinMaxRange.Y;
    out.BaseWeaponCriticalMultiplier = ws->BaseWeaponCriticalMultiplier.BaseValue;
    out.BaseWeaponCriticalMultiplierMinRange = ws->BaseWeaponCriticalMultiplier.MinMaxRange.X;
    out.BaseWeaponCriticalMultiplierMaxRange = ws->BaseWeaponCriticalMultiplier.MinMaxRange.Y;
    out.BaseRateOfFire = ws->BaseRateOfFire.BaseValue;
    out.BaseRateOfFireMinRange = ws->BaseRateOfFire.MinMaxRange.X;
    out.BaseRateOfFireMaxRange = ws->BaseRateOfFire.MinMaxRange.Y;
    out.BaseCooldown = ws->BaseCooldown.BaseValue;
    out.BaseCooldownMinRange = ws->BaseCooldown.MinMaxRange.X;
    out.BaseCooldownMaxRange = ws->BaseCooldown.MinMaxRange.Y;
    out.BaseRecoil = ws->BaseRecoil.BaseValue;
    out.BaseRecoilMinRange = ws->BaseRecoil.MinMaxRange.X;
    out.BaseRecoilMaxRange = ws->BaseRecoil.MinMaxRange.Y;
    out.BaseRecoilRecovery = ws->BaseRecoilRecovery.BaseValue;
    out.BaseRecoilRecoveryMinRange = ws->BaseRecoilRecovery.MinMaxRange.X;
    out.BaseRecoilRecoveryMaxRange = ws->BaseRecoilRecovery.MinMaxRange.Y;
    out.BaseSpread = ws->BaseSpread.BaseValue;
    out.BaseSpreadMinRange = ws->BaseSpread.MinMaxRange.X;
    out.BaseSpreadMaxRange = ws->BaseSpread.MinMaxRange.Y;
    out.BaseReloadTime = ws->BaseReloadTime.BaseValue;
    out.BaseReloadTimeMinRange = ws->BaseReloadTime.MinMaxRange.X;
    out.BaseReloadTimeMaxRange = ws->BaseReloadTime.MinMaxRange.Y;
    out.ReloadTimeDelta = ws->ReloadTimeDelta;
    out.isValid = true;
}


} // namespace Features
} // namespace Cheat

