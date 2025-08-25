#include "SaveService.h"
#include "../Services/GameServices.h"
#include <dev/logger.h>

namespace Cheat {
namespace Features {

static inline SDK::URSaveGame* GetCurrentSave() {
    auto* world = Services::GameServices::GetWorld();
    if (!world) return nullptr;
    auto* gi = SDK::UGameplayStatics::GetGameInstance(world);
    if (!gi) return nullptr;
    // Downcast to URGameInstance to access CurrentSaveGame
    auto* rgi = reinterpret_cast<SDK::URGameInstance*>(gi);
    if (!rgi) return nullptr;
    return rgi->CurrentSaveGame;
}

bool SaveService::TryGetSoulFragments(int32_t& outSoulFragments) {
    auto* save = GetCurrentSave();
    if (!save) return false;
    outSoulFragments = save->SaveGameData.SoulFragments;
    return true;
}

bool SaveService::IncrementSoulFragments(int32_t delta) {
    auto* save = GetCurrentSave();
    if (!save) return false;
    save->IncrementSoulFragments(delta);
    LOG_INFO("SaveService: Incremented SoulFragments by %d -> now %d", delta, save->SaveGameData.SoulFragments);
    return true;
}

bool SaveService::SetSoulFragments(int32_t value) {
    auto* save = GetCurrentSave();
    if (!save) return false;
    int32_t current = save->SaveGameData.SoulFragments;
    int32_t delta = value - current;
    if (delta != 0) {
        if (delta > 0) save->IncrementSoulFragments(delta);
        else           save->DecrementSoulFragments(-delta);
    }
    LOG_INFO("SaveService: Set SoulFragments %d -> %d (delta=%d)", current, value, delta);
    return true;
}

} // namespace Features
} // namespace Cheat

