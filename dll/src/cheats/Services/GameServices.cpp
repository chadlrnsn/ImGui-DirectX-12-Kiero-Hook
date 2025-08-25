#include "GameServices.h"
#include <iostream>
#include <dev/logger.h>

namespace Cheat {
namespace Services {

bool GameServices::Refresh(bool log) {
    // World
    s_world = SDK::UWorld::GetWorld();
    if (!s_world || !s_world->GameState) {
        if (log) std::cerr << "Error: World not found" << std::endl;
        return false;
    }
    if (log) {
        std::cout << "World address: 0x" << std::hex << reinterpret_cast<uintptr_t>(s_world) << std::dec << std::endl;
    }

    // Engine
    s_engine = SDK::UEngine::GetEngine();
    if (!s_engine) {
        if (log) std::cerr << "Error: Engine not found" << std::endl;
        return false;
    }
    if (log) {
        std::cout << "Engine address: 0x" << std::hex << reinterpret_cast<uintptr_t>(s_engine) << std::dec << std::endl;
    }

    // Controller + Pawn
    if (s_world->OwningGameInstance && s_world->OwningGameInstance->LocalPlayers.Num() > 0) {
        s_controller = s_world->OwningGameInstance->LocalPlayers[0]->PlayerController;
        if (log) {
            std::cout << "PlayerController address: 0x" << std::hex << reinterpret_cast<uintptr_t>(s_controller) << std::dec << std::endl;
        }
        if (s_controller) {
            s_pawn = s_controller->K2_GetPawn();
            s_rpawn = static_cast<SDK::ARPlayerPawn*>(s_pawn);
            if (log) {
                std::cout << "MyPawn address: 0x" << std::hex << reinterpret_cast<uintptr_t>(s_pawn) << std::dec << std::endl;
                if (s_pawn) {
                    std::cout << "MyPawn name: " << s_pawn->GetName() << std::endl;
                }
            }
        }
    }

    // No longer backfilling Config::GameState with core pointers; use accessors
    UpdateWeaponCache(s_rpawn);
    return true;
}

void GameServices::UpdateWeaponCache(SDK::ARPlayerPawn* pawn) {
    if (!pawn) {
        s_weapon = nullptr;
        s_weaponScript = nullptr;
        s_engineRifleScript = nullptr;
        s_isEngineRifle = false;

        // no legacy globals update; runtime cache is owned here
        return;
    }

    auto newWeapon = pawn->GetEquippedWeapon();
    if (newWeapon != s_weapon) {
        s_weapon = newWeapon;
        s_weaponScript = nullptr;
        s_engineRifleScript = nullptr;
        s_isEngineRifle = false;

        // reset our runtime caches

        if (s_weapon && s_weapon->RuntimeWeaponScript) {
            s_weaponScript = s_weapon->RuntimeWeaponScript;
            if (s_weaponScript->IsA(SDK::UBP_EngineRifle_Script_C::StaticClass())) {
                s_engineRifleScript = static_cast<SDK::UBP_EngineRifle_Script_C*>(s_weaponScript);
                s_isEngineRifle = true;
                LOG_INFO("[WEAPON] Engine Rifle detected and cached!");
            }
        }
    }
}

SDK::UEngine* GameServices::GetEngine() { return s_engine; }
SDK::UWorld* GameServices::GetWorld() { return s_world; }
SDK::APlayerController* GameServices::GetPlayerController() { return s_controller; }
SDK::APawn* GameServices::GetPlayerPawn() { return s_pawn; }
SDK::ARPlayerPawn* GameServices::GetRPlayerPawn() { return s_rpawn; }
SDK::ARWeapon* GameServices::GetEquippedWeapon() { return s_weapon; }
SDK::URGWeaponScript* GameServices::GetWeaponScript() { return s_weaponScript; }
SDK::UBP_EngineRifle_Script_C* GameServices::GetEngineRifleScript() { return s_engineRifleScript; }
bool GameServices::IsEngineRifle() { return s_isEngineRifle; }

} // namespace Services
} // namespace Cheat

