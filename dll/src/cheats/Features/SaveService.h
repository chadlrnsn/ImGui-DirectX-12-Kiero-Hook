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

    // Set soul fragments to an absolute value; returns false if chain is invalid
    static bool SetSoulFragments(int32_t value);
};

} // namespace Features
} // namespace Cheat

