#include "AimbotService.h"
#include "../Aimbot/AimbotController.h"

namespace Cheat { namespace Services {

void AimbotService::Initialize() {
    AimbotController::Initialize();
}

void AimbotService::Update(float deltaTime) {
    AimbotController::Update(deltaTime);
}

void AimbotService::Shutdown() {
    AimbotController::Shutdown();
}

} } // namespace Cheat::Services

