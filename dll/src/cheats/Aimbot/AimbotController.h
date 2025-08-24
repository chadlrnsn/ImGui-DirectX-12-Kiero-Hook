#pragma once

#include "../SDK/Engine_classes.hpp"
#include "../SDK/RGame_classes.hpp"
#include "../Core/Config.h"
#include "TargetSelector.h"

class AimbotController {
public:
    static void Update(float deltaTime);
    static void Initialize();
    static void Shutdown();
    
    // Input handling
    static bool IsAimbotKeyPressed();
    
    // Core aimbot functionality
    static void ProcessAiming(SDK::UWorld* world, 
                            SDK::APlayerController* playerController,
                            SDK::ARPlayerPawn* playerPawn,
                            float deltaTime);
    
private:
    static DWORD m_lastUpdateTime;
    
    // Advanced aiming methods
    static void ApplySmoothing(SDK::APlayerController* playerController,
                             const SDK::FRotator& targetRotation,
                             float deltaTime,
                             bool shouldLog = false);

    static void ApplySmoothAiming(SDK::APlayerController* playerController,
                                const SDK::FVector& targetWorldPosition,
                                float deltaTime,
                                bool shouldLog = false);

    static void ApplyHumanizedAiming(SDK::APlayerController* playerController,
                                   const SDK::FRotator& targetRotation,
                                   float deltaTime,
                                   bool shouldLog = false);
    
    // Validation
    static bool ValidateGameState(SDK::UWorld* world, 
                                SDK::APlayerController* playerController,
                                SDK::ARPlayerPawn* playerPawn);
};