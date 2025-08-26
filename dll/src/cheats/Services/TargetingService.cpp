#include "TargetingService.h"
#include "../Aimbot/TargetSelector.h"

namespace Cheat { namespace Services {

TargetInfo TargetingService::GetBestTarget(SDK::UWorld* world,
                                           SDK::APlayerController* controller,
                                           SDK::ARPlayerPawn* pawn) {
    return TargetSelector::SelectBestTarget(world, controller, pawn);
}

} } // namespace Cheat::Services

