#pragma once

#include "../SDK/Engine_classes.hpp"
#include "../SDK/RGame_classes.hpp"

namespace Cheat { namespace Services {

class UnlockService {
public:
    // Lifecycle
    static void Initialize();
    static void Shutdown();

    // Individual unlock actions
    static void UnlockAllAbilities();
    static void UnlockAllLockedContent();
    static void UnlockAllMutators();
    static void UnlockAllWeaponMods();

    // Convenience aggregator
    static void UnlockAll();
};

} } // namespace Cheat::Services

