#pragma once

#include "../SDK/Engine_classes.hpp"
#include "../SDK/RGame_classes.hpp"
#include "../SDK/BP_EngineRifle_Script_classes.hpp"
#include "../Core/Config.h"

namespace Cheat {
namespace Services {

class GameServices {
public:
    // Refresh cached pointers to engine/world/controller/pawn/weapon. Returns true if world+engine found
    static bool Refresh(bool log = false);

    // Accessors
    static SDK::UEngine* GetEngine();
    static SDK::UWorld* GetWorld();
    static SDK::APlayerController* GetPlayerController();
    static SDK::APawn* GetPlayerPawn();
    static SDK::ARPlayerPawn* GetRPlayerPawn();
    static SDK::ARWeapon* GetEquippedWeapon();
    static SDK::URGWeaponScript* GetWeaponScript();
    static SDK::UBP_EngineRifle_Script_C* GetEngineRifleScript();
    static bool IsEngineRifle();

private:
    static void UpdateWeaponCache(SDK::ARPlayerPawn* pawn);

    // Cached pointers
    static inline SDK::UEngine* s_engine = nullptr;
    static inline SDK::UWorld* s_world = nullptr;
    static inline SDK::APlayerController* s_controller = nullptr;
    static inline SDK::APawn* s_pawn = nullptr;
    static inline SDK::ARPlayerPawn* s_rpawn = nullptr;

    static inline SDK::ARWeapon* s_weapon = nullptr;
    static inline SDK::URGWeaponScript* s_weaponScript = nullptr;
    static inline SDK::UBP_EngineRifle_Script_C* s_engineRifleScript = nullptr;
    static inline bool s_isEngineRifle = false;
};

} // namespace Services
} // namespace Cheat

