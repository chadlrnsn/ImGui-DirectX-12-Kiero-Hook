#include "ResourceService.h"
#include <dev/logger.h>

namespace Cheat { namespace Features {

static inline SDK::ARPlayerState* GetRPlayerState() {
    auto* world = Services::GameServices::GetWorld();
    if (!world) return nullptr;
    auto* playerState = SDK::UGameplayStatics::GetPlayerState(world, 0);
    if (!playerState) return nullptr;
    return static_cast<SDK::ARPlayerState*>(playerState);
}

bool ResourceService::AddGold(int32_t amount) {
    if (amount == 0) return true;
    if (auto* rps = GetRPlayerState()) {
        rps->AddGold(amount);
        LOG_INFO("RunResourceService: Added gold: %d", amount);
        return true;
    }
    return false;
}

bool ResourceService::AddKeys(int32_t amount) {
    if (amount == 0) return true;
    if (auto* rps = GetRPlayerState()) {
        rps->AddKeys(amount);
        LOG_INFO("RunResourceService: Added keys: %d", amount);
        return true;
    }
    return false;
}

} } // namespace Cheat::Features

