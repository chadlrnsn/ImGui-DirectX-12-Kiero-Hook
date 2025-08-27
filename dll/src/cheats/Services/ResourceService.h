#pragma once

#include "../SDK/Engine_classes.hpp"
#include "../SDK/RGame_classes.hpp"
#include "GameServices.h"

namespace Cheat { namespace Services {

// Handles temporary, in-run resources such as Gold and Keys
class ResourceService {
public:
    static bool AddGold(int32_t amount);
    static bool AddKeys(int32_t amount);
};

} } // namespace Cheat::Services

