#pragma once

#include "../SDK/Engine_classes.hpp"
#include "../SDK/RGame_classes.hpp"

namespace Cheat {
namespace Features {

class SaveService {
public:
    // Query current Soul Fragments; returns false if chain is invalid
    static bool TryGetSoulFragments(int32_t& outSoulFragments);

    // Increment soul fragments by delta; returns false if chain is invalid
    static bool IncrementSoulFragments(int32_t delta);
};

} // namespace Features
} // namespace Cheat

