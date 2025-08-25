#include "AimbotController.h"
#include "MathUtils.h"
#include "../Core/Config.h"
#include "../Services/GameServices.h"
#include <dev/validity.h>
#include <dev/logger.h>
#include <Windows.h>
#include <iomanip>

DWORD AimbotController::m_lastUpdateTime = 0;

void AimbotController::Initialize() {
    m_lastUpdateTime = GetTickCount();
    Cheat::Config::GameState::g_pCurrentTarget = nullptr;
    Cheat::Config::GameState::g_CurrentTargetInfo = TargetInfo();
    std::cout << "[AIMBOT] Initialized successfully" << std::endl;
}

void AimbotController::Shutdown() {
    Cheat::Config::GameState::g_pCurrentTarget = nullptr;
    Cheat::Config::GameState::g_CurrentTargetInfo = TargetInfo();
    std::cout << "[AIMBOT] Shutdown completed" << std::endl;
}

void AimbotController::Update(float deltaTime) {
    static DWORD lastLogTime = 0;
    DWORD currentTime = GetTickCount();
    bool shouldLog = (currentTime - lastLogTime) > 1000; // Log every second for basic status

    // Check for toggle key
    static bool toggleKeyWasPressed = false;
    bool toggleKeyPressed = (GetAsyncKeyState(Cheat::Config::Hotkeys::AimbotToggle) & 0x8000) != 0;
    if (toggleKeyPressed && !toggleKeyWasPressed) {
        Cheat::Config::Aimbot::enabled = !Cheat::Config::Aimbot::enabled;
        LOG_INFO("HOTKEY: Aimbot %s (F2 pressed)", Cheat::Config::Aimbot::enabled ? "ENABLED" : "DISABLED");
    }
    toggleKeyWasPressed = toggleKeyPressed;

    if (!Cheat::Config::Aimbot::enabled) {
        if (shouldLog) {
            lastLogTime = currentTime;
        }
        return;
    }

    // Use GameServices for core objects
    SDK::UWorld* world = Cheat::Services::GameServices::GetWorld();
    SDK::APlayerController* playerController = Cheat::Services::GameServices::GetPlayerController();
    SDK::ARPlayerPawn* playerPawn = Cheat::Services::GameServices::GetRPlayerPawn();

    // Debug: Check if centralized state is available
    // Optionally: add lightweight debug here if needed

    // Validation
    if (!ValidateGameState(world, playerController, playerPawn)) {
        if (shouldLog) {
            lastLogTime = currentTime;
        }
        return;
    }

    // Update lastLogTime after successful validation to prevent log spam
    if (shouldLog) {
        lastLogTime = currentTime;
    }
    

    
    // Process aimbot logic
    ProcessAiming(world, playerController, playerPawn, deltaTime);
}

bool AimbotController::IsAimbotKeyPressed() {
    bool isPressed = (GetAsyncKeyState(Cheat::Config::Hotkeys::AimbotTrigger) & 0x8000) != 0;
    static bool lastKeyState = false;
    static DWORD lastKeyLogTime = 0;
    DWORD currentTime = GetTickCount();
    
    // Log key state changes
    if (isPressed != lastKeyState) {
        lastKeyState = isPressed;
        lastKeyLogTime = currentTime;
    }
    
    return isPressed;
}

void AimbotController::ProcessAiming(SDK::UWorld* world, 
                                   SDK::APlayerController* playerController,
                                   SDK::ARPlayerPawn* playerPawn,
                                   float deltaTime) {
    static DWORD lastDetailedLogTime = 0;
    DWORD currentTime = GetTickCount();
    bool shouldLogDetailed = (currentTime - lastDetailedLogTime) > 500; // Detailed logs every 500ms when aiming
    
    // Only aim when key is pressed
    if (!IsAimbotKeyPressed()) {
        static bool wasAiming = false;
        if (wasAiming) {
            wasAiming = false;
        }
        Cheat::Config::GameState::g_CurrentTargetInfo = TargetInfo(); // Clear target
        Cheat::Config::GameState::g_pCurrentTarget = nullptr;
        return;
    }

    // Get player location (needed for calculations)
    SDK::FVector playerLocation = playerPawn->K2_GetActorLocation();



    // Find the best target
    TargetInfo newTarget = TargetSelector::SelectBestTarget(world, playerController, playerPawn);
    
    // Log target search results
    if (!newTarget.actor) {
        if (shouldLogDetailed) {
            lastDetailedLogTime = currentTime;
        }
        Cheat::Config::GameState::g_CurrentTargetInfo = TargetInfo();
        Cheat::Config::GameState::g_pCurrentTarget = nullptr;
        return;
    }


    
    // If we found a valid target
    if (newTarget.actor) {
        // Apply target switching delay for more human-like behavior
        static DWORD lastTargetSwitch = 0;
        if (Cheat::Config::GameState::g_CurrentTargetInfo.actor != newTarget.actor) {
            DWORD currentTime = GetTickCount();
            if (currentTime - lastTargetSwitch < (Cheat::Config::Aimbot::targetSwitchDelay * 1000)) {
                // Too soon to switch targets, keep current one if still valid
                if (Cheat::Config::GameState::g_CurrentTargetInfo.actor) {

                    newTarget = Cheat::Config::GameState::g_CurrentTargetInfo;
                } else {

                    return;
                }
            } else {

                lastTargetSwitch = currentTime;
            }
        }
        
        Cheat::Config::GameState::g_CurrentTargetInfo = newTarget;
        Cheat::Config::GameState::g_pCurrentTarget = newTarget.actor;

        // For bone-based targeting, we can use the precise bone location directly
        SDK::FVector targetPosition = Cheat::Config::GameState::g_CurrentTargetInfo.aimPoint;
        
        // Calculate aim rotation
        SDK::FVector directionToTarget = Math::Normalize(Math::Subtract(targetPosition, playerLocation));
        SDK::FRotator targetRotation = Math::VectorToRotation(directionToTarget);
        SDK::FRotator currentRotation = playerController->K2_GetActorRotation();
        
        // Calculate angular distance (used later)
        float angularDistance = Math::GetAngleBetweenRotations(currentRotation, targetRotation);
        
if (Cheat::Config::Aimbot::smoothEnabled) {

            ApplySmoothing(playerController, targetRotation, deltaTime, shouldLogDetailed);
        } else {
            // Instant snap to target
            playerController->SetControlRotation(targetRotation);
        }
    } else {
        // No valid target found
        Cheat::Config::GameState::g_CurrentTargetInfo = TargetInfo();
        Cheat::Config::GameState::g_pCurrentTarget = nullptr;
    }

    // Always update the log time to prevent spam
    if (shouldLogDetailed) {
        lastDetailedLogTime = currentTime;
    }
}

void AimbotController::ApplySmoothing(SDK::APlayerController* playerController,
                                    const SDK::FRotator& targetRotation,
                                    float deltaTime,
                                    bool shouldLog) {
    SDK::FRotator currentRotation = playerController->K2_GetActorRotation();



    // Calculate smooth rotation using lerp
    SDK::FRotator smoothedRotation = Math::LerpRotation(
        currentRotation,
        targetRotation,
        deltaTime,
        Cheat::Config::Aimbot::maxTurnSpeed
    );



    // Apply the smoothed rotation
    playerController->SetControlRotation(smoothedRotation);


}

void AimbotController::ApplySmoothAiming(SDK::APlayerController* playerController,
                                       const SDK::FVector& targetWorldPosition,
                                       float deltaTime,
                                       bool shouldLog) {
    SDK::FRotator currentRotation = playerController->K2_GetActorRotation();
    
    // Get camera position
    SDK::FVector cameraPosition;
    SDK::FRotator cameraRotation;
    playerController->GetActorEyesViewPoint(&cameraPosition, &cameraRotation);
    

    
    // Calculate the rotation needed to look at the target using UKismetMathLibrary
    SDK::FRotator targetRotation = Math::CalculateLookAtRotation(cameraPosition, targetWorldPosition);
    
    // Use UKismetMathLibrary for smooth rotation interpolation
    SDK::FRotator smoothedRotation = Math::LerpRotation(
        currentRotation, 
        targetRotation, 
        deltaTime, 
        Cheat::Config::Aimbot::maxTurnSpeed
    );
    
    // Apply the smoothed rotation
    playerController->SetControlRotation(smoothedRotation);
}

void AimbotController::ApplyHumanizedAiming(SDK::APlayerController* playerController,
                                          const SDK::FRotator& targetRotation,
                                          float deltaTime,
                                          bool shouldLog) {
    SDK::FRotator currentRotation = playerController->K2_GetActorRotation();
    
    // Calculate angular distance using UKismetMathLibrary
    float angularDistance = Math::GetAngleBetweenRotations(currentRotation, targetRotation);
    

    
    // Apply reaction time delay
    static DWORD lastReactionTime = 0;
    DWORD currentTime = GetTickCount();
    if (currentTime - lastReactionTime < (Cheat::Config::Aimbot::reactionTime * 1000)) {

        return; // Still in reaction period
    }
    
    // Determine aiming speed based on distance
    float aimSpeed = Cheat::Config::Aimbot::maxTurnSpeed;

    // For large distances, use instant snap (simulating mouse flick)
    if (angularDistance > Cheat::Config::Aimbot::maxAimSnapDistance) {

        playerController->SetControlRotation(targetRotation);
        lastReactionTime = currentTime;
        return;
    }
    
    // Add slight randomness to movement speed for human-like behavior
    float randomFactor = 0.8f + (rand() % 40) / 100.0f; // 0.8 to 1.2
    aimSpeed *= randomFactor;
    

    
    // Apply smooth rotation using UKismetMathLibrary
    SDK::FRotator smoothedRotation = Math::LerpRotation(
        currentRotation, 
        targetRotation, 
        deltaTime, 
        aimSpeed
    );
    
    playerController->SetControlRotation(smoothedRotation);

}

bool AimbotController::ValidateGameState(SDK::UWorld* world,
                                       SDK::APlayerController* playerController,
                                       SDK::ARPlayerPawn* playerPawn) {
    // Basic null checks first
    if (!world) {
        std::cout << "[AIMBOT] Validation failed: World is null" << std::endl;
        return false;
    }
    if (!playerController) {
        std::cout << "[AIMBOT] Validation failed: PlayerController is null" << std::endl;
        return false;
    }
    if (!playerPawn) {
        std::cout << "[AIMBOT] Validation failed: PlayerPawn is null" << std::endl;
        return false;
    }

    // Try to access PersistentLevel safely
    __try {
        if (!world->PersistentLevel) {
            std::cout << "[AIMBOT] Validation failed: PersistentLevel is null" << std::endl;
            return false;
        }
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        std::cout << "[AIMBOT] Validation failed: Exception accessing PersistentLevel" << std::endl;
        return false;
    }

    // Check if player is alive using the GetIsAlive method
    __try {
        if (!playerPawn->GetIsAlive()) {
            std::cout << "[AIMBOT] Validation failed: Player is not alive" << std::endl;
            return false;
        }
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        std::cout << "[AIMBOT] Validation failed: Exception accessing player alive state" << std::endl;
        return false;
    }

    return true;
}