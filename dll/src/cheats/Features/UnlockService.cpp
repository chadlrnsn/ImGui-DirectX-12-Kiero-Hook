#include "UnlockService.h"
#include "../Services/GameServices.h"
#include <dev/logger.h>

namespace Cheat {
namespace Features {

void UnlockService::Initialize() {
    LOG_INFO("UnlockService initialized");
}

void UnlockService::Shutdown() {
    LOG_INFO("UnlockService shutdown");
}

static inline SDK::UWorld* WorldCtx() {
    return Cheat::Services::GameServices::GetWorld();
}

void UnlockService::UnlockAllAbilities() {
    if (auto* world = WorldCtx()) {
        SDK::URGameFunctionLibrary::UnlockAllAbilities(world);
        LOG_INFO("UnlockService: UnlockAllAbilities() called");
    } else {
        LOG_ERROR("UnlockService: World is null for UnlockAllAbilities");
    }
}

void UnlockService::UnlockAllLockedContent() {
    if (auto* world = WorldCtx()) {
        SDK::URGameFunctionLibrary::UnlockAllLockedContent(world);
        LOG_INFO("UnlockService: UnlockAllLockedContent() called");
    } else {
        LOG_ERROR("UnlockService: World is null for UnlockAllLockedContent");
    }
}

void UnlockService::UnlockAllMutators() {
    if (auto* world = WorldCtx()) {
        SDK::URGameFunctionLibrary::UnlockAllMutators(world);
        LOG_INFO("UnlockService: UnlockAllMutators() called");
    } else {
        LOG_ERROR("UnlockService: World is null for UnlockAllMutators");
    }
}

void UnlockService::UnlockAllWeaponMods() {
    if (auto* world = WorldCtx()) {
        SDK::URGameFunctionLibrary::UnlockAllWeaponMods(world);
        LOG_INFO("UnlockService: UnlockAllWeaponMods() called");
    } else {
        LOG_ERROR("UnlockService: World is null for UnlockAllWeaponMods");
    }
}

void UnlockService::UnlockAll() {
    UnlockAllAbilities();
    UnlockAllLockedContent();
    UnlockAllMutators();
    UnlockAllWeaponMods();
}

} // namespace Features
} // namespace Cheat

