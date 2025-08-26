#pragma once

#include "../SDK/Engine_classes.hpp"
#include "../SDK/RGame_classes.hpp"

namespace Cheat { namespace Services {

class AimbotService {
public:
    // Lifecycle
    static void Initialize();
    static void Update(float deltaTime);
    static void Shutdown();
};

} } // namespace Cheat::Services

