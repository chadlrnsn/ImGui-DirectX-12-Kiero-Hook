#pragma once

#include "../SDK/Engine_classes.hpp"
#include "../SDK/RGame_classes.hpp"
#include "../Core/Config.h"

namespace Cheat {
namespace Services {

class PlayerEffectsService {
public:
    // Apply gameplay effects such as God Mode, Speed Hack, Movement hacks; ensure CheatManager if needed
    static void Update(SDK::APlayerController* playerController);
};

} // namespace Services
} // namespace Cheat

