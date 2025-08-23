#include "AimbotController.h"
#include "MathUtils.h"
#include "../Core/Config.h"
#include <Windows.h>
#include <iomanip>

TargetInfo AimbotController::m_currentTarget;
DWORD AimbotController::m_lastUpdateTime = 0;

void AimbotController::Initialize() {
    m_lastUpdateTime = GetTickCount();
    m_currentTarget = TargetInfo();
    std::cout << "[AIMBOT] Initialized successfully" << std::endl;
}

void AimbotController::Shutdown() {
    m_currentTarget = TargetInfo();
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
		std::cout << "[AIMBOT] Toggle pressed - Aimbot " << (Cheat::Config::Aimbot::enabled ? "ENABLED" : "DISABLED") << std::endl;
    }
    toggleKeyWasPressed = toggleKeyPressed;
    
    if (!Cheat::Config::Aimbot::enabled) {
        if (shouldLog) {
            std::cout << "[AIMBOT] Status: DISABLED - skipping update" << std::endl;
            lastLogTime = currentTime;
        }
        return;
    }
    

    
    // Get game state from centralized config
    SDK::UWorld* world = Cheat::Config::GameState::g_pWorld;
    if (!world || !world->OwningGameInstance) {
        if (shouldLog) {
            std::cout << "[AIMBOT] ERROR: World or OwningGameInstance is null" << std::endl;
            lastLogTime = currentTime;
        }
        return;
    }
    
    auto localPlayer = world->OwningGameInstance->LocalPlayers[0];
    if (!localPlayer) {
        if (shouldLog) {
            std::cout << "[AIMBOT] ERROR: LocalPlayer is null" << std::endl;
            lastLogTime = currentTime;
        }
        return;
    }
    
    auto playerController = localPlayer->PlayerController;
    if (!playerController) {
        if (shouldLog) {
            std::cout << "[AIMBOT] ERROR: PlayerController is null" << std::endl;
            lastLogTime = currentTime;
        }
        return;
    }
    
    auto playerPawn = static_cast<SDK::ARPlayerPawn*>(playerController->K2_GetPawn());
    if (!ValidateGameState(world, playerController, playerPawn)) {
        if (shouldLog) {
            std::cout << "[AIMBOT] ERROR: Game state validation failed" << std::endl;
            lastLogTime = currentTime;
        }
        return;
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
        std::cout << "[AIMBOT] Trigger key (Mouse4) " << (isPressed ? "PRESSED" : "RELEASED") << std::endl;
        lastKeyState = isPressed;
        lastKeyLogTime = currentTime;
    }
    // Also log periodically if key is held down
    else if (isPressed && (currentTime - lastKeyLogTime) > 2000) {
        std::cout << "[AIMBOT] Trigger key still held down..." << std::endl;
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
            std::cout << "[AIMBOT] Trigger key released - clearing target" << std::endl;
            wasAiming = false;
        }
        m_currentTarget = TargetInfo(); // Clear target
        return;
    }
    
    std::cout << "[AIMBOT] Processing aiming..." << std::endl;
    
    // Log player position for reference
    SDK::FVector playerLocation = playerPawn->K2_GetActorLocation();
    std::cout << "[AIMBOT] Player location: (" << std::fixed << std::setprecision(1) 
              << playerLocation.X << ", " << playerLocation.Y << ", " << playerLocation.Z << ")" << std::endl;
    
    // Find the best target
    std::cout << "[AIMBOT] Searching for targets..." << std::endl;
    TargetInfo newTarget = TargetSelector::SelectBestTarget(world, playerController, playerPawn);
    
    // Log target search results
    if (!newTarget.actor) {
        std::cout << "[AIMBOT] No valid targets found!" << std::endl;
        std::cout << "[AIMBOT] - Max distance: " << Cheat::Config::Aimbot::maxDistance << " units" << std::endl;
        std::cout << "[AIMBOT] - FOV radius: " << Cheat::Config::Aimbot::fovRadius << " degrees" << std::endl;
        std::cout << "[AIMBOT] - Visibility check: " << (Cheat::Config::Aimbot::visibilityCheck ? "enabled" : "disabled") << std::endl;
        m_currentTarget = TargetInfo();
        lastDetailedLogTime = currentTime;
        return;
    }
    
    std::cout << "[AIMBOT] Target found!" << std::endl;
    std::cout << "[AIMBOT] - Target name: " << newTarget.actor->GetName() << std::endl;
    std::cout << "[AIMBOT] - Distance: " << std::fixed << std::setprecision(1) << newTarget.distance << " units" << std::endl;
    std::cout << "[AIMBOT] - FOV distance: " << std::fixed << std::setprecision(1) << newTarget.fovDistance << " pixels" << std::endl;
    std::cout << "[AIMBOT] - Visible: " << (newTarget.isVisible ? "yes" : "no") << std::endl;
    std::cout << "[AIMBOT] - Aim point: (" << std::fixed << std::setprecision(1) 
              << newTarget.aimPoint.X << ", " << newTarget.aimPoint.Y << ", " << newTarget.aimPoint.Z << ")" << std::endl;
    
    // Log bone targeting information
    if (newTarget.hasBoneTarget) {
        std::cout << "[AIMBOT] - Using BONE-BASED targeting: " << newTarget.targetBoneName 
                  << " (index " << newTarget.targetBoneIndex << ")" << std::endl;
    } else {
        std::cout << "[AIMBOT] - Using FALLBACK targeting (no bone data available)" << std::endl;
    }
    
    // If we found a valid target
    if (newTarget.actor) {
        // Apply target switching delay for more human-like behavior
        static DWORD lastTargetSwitch = 0;
        if (m_currentTarget.actor != newTarget.actor) {
            DWORD currentTime = GetTickCount();
            if (currentTime - lastTargetSwitch < (Cheat::Config::Aimbot::targetSwitchDelay * 1000)) {
                // Too soon to switch targets, keep current one if still valid
                if (m_currentTarget.actor) {
                    std::cout << "[AIMBOT] Target switch delay active - keeping current target" << std::endl;
                    newTarget = m_currentTarget;
                } else {
                    std::cout << "[AIMBOT] Target switch delay active - no current target, skipping" << std::endl;
                    return;
                }
            } else {
                std::cout << "[AIMBOT] Switching to new target: " << newTarget.actor->GetName() << std::endl;
                if (newTarget.hasBoneTarget) {
                    std::cout << "[AIMBOT] - New target bone: " << newTarget.targetBoneName 
                              << " (index " << newTarget.targetBoneIndex << ")" << std::endl;
                }
                lastTargetSwitch = currentTime;
            }
        }
        
        m_currentTarget = newTarget;
        
        // For bone-based targeting, we can use the precise bone location directly
        SDK::FVector targetPosition = m_currentTarget.aimPoint;
        
        // Calculate aim rotation
        SDK::FVector directionToTarget = Math::Normalize(Math::Subtract(targetPosition, playerLocation));
        SDK::FRotator targetRotation = Math::VectorToRotation(directionToTarget);
        SDK::FRotator currentRotation = playerController->K2_GetActorRotation();
        
        // Log aiming calculations
        std::cout << "[AIMBOT] Aiming calculations:" << std::endl;
        std::cout << "[AIMBOT] - Current rotation: (" << std::fixed << std::setprecision(1) 
                  << currentRotation.Pitch << ", " << currentRotation.Yaw << ", " << currentRotation.Roll << ")" << std::endl;
        std::cout << "[AIMBOT] - Target rotation: (" << std::fixed << std::setprecision(1) 
                  << targetRotation.Pitch << ", " << targetRotation.Yaw << ", " << targetRotation.Roll << ")" << std::endl;
        
        float angularDistance = Math::GetAngleBetweenRotations(currentRotation, targetRotation);
        std::cout << "[AIMBOT] - Angular distance: " << std::fixed << std::setprecision(1) << angularDistance << " degrees" << std::endl;
        
        if (m_currentTarget.hasBoneTarget) {
            std::cout << "[AIMBOT] - Targeting precise bone location for " << m_currentTarget.targetBoneName << std::endl;
        }
        
        // Apply humanized aiming
        if (Cheat::Config::Aimbot::humanizeMovement) {
            std::cout << "[AIMBOT] Applying humanized aiming..." << std::endl;
            ApplyHumanizedAiming(playerController, targetRotation, deltaTime);
        } else if (Cheat::Config::Aimbot::smoothEnabled) {
            std::cout << "[AIMBOT] Applying smooth aiming..." << std::endl;
            ApplySmoothing(playerController, targetRotation, deltaTime);
        } else {
            std::cout << "[AIMBOT] Applying instant snap to target" << std::endl;
            // Instant snap to target
            playerController->SetControlRotation(targetRotation);
        }
    } else {
        // No valid target found
        std::cout << "[AIMBOT] Lost target - clearing current target" << std::endl;
        m_currentTarget = TargetInfo();
    }
    
    lastDetailedLogTime = currentTime;
}

void AimbotController::ApplySmoothing(SDK::APlayerController* playerController,
                                    const SDK::FRotator& targetRotation,
                                    float deltaTime) {
    SDK::FRotator currentRotation = playerController->K2_GetActorRotation();
    
    std::cout << "[AIMBOT] Smooth aiming parameters:" << std::endl;
    std::cout << "[AIMBOT] - Max turn speed: " << Cheat::Config::Aimbot::maxTurnSpeed << " deg/s" << std::endl;
    std::cout << "[AIMBOT] - Delta time: " << std::fixed << std::setprecision(3) << deltaTime << "s" << std::endl;
    
    // Calculate smooth rotation using lerp
    SDK::FRotator smoothedRotation = Math::LerpRotation(
        currentRotation, 
        targetRotation, 
        deltaTime, 
        Cheat::Config::Aimbot::maxTurnSpeed
    );
    
    std::cout << "[AIMBOT] - Smoothed rotation: (" << std::fixed << std::setprecision(1) 
              << smoothedRotation.Pitch << ", " << smoothedRotation.Yaw << ", " << smoothedRotation.Roll << ")" << std::endl;
    
    // Apply the smoothed rotation
    playerController->SetControlRotation(smoothedRotation);
    std::cout << "[AIMBOT] Smooth rotation applied" << std::endl;
}

void AimbotController::ApplySmoothAiming(SDK::APlayerController* playerController,
                                       const SDK::FVector& targetWorldPosition,
                                       float deltaTime) {
    SDK::FRotator currentRotation = playerController->K2_GetActorRotation();
    
    // Get camera position
    SDK::FVector cameraPosition;
    SDK::FRotator cameraRotation;
    playerController->GetActorEyesViewPoint(&cameraPosition, &cameraRotation);
    
    std::cout << "[AIMBOT] Camera-based smooth aiming:" << std::endl;
    std::cout << "[AIMBOT] - Camera position: (" << std::fixed << std::setprecision(1) 
              << cameraPosition.X << ", " << cameraPosition.Y << ", " << cameraPosition.Z << ")" << std::endl;
    
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
    std::cout << "[AIMBOT] Camera-based smooth rotation applied" << std::endl;
}

void AimbotController::ApplyHumanizedAiming(SDK::APlayerController* playerController,
                                          const SDK::FRotator& targetRotation,
                                          float deltaTime) {
    SDK::FRotator currentRotation = playerController->K2_GetActorRotation();
    
    // Calculate angular distance using UKismetMathLibrary
    float angularDistance = Math::GetAngleBetweenRotations(currentRotation, targetRotation);
    
    std::cout << "[AIMBOT] Humanized aiming:" << std::endl;
    std::cout << "[AIMBOT] - Angular distance: " << std::fixed << std::setprecision(1) << angularDistance << " degrees" << std::endl;
    std::cout << "[AIMBOT] - Reaction time: " << Cheat::Config::Aimbot::reactionTime << "s" << std::endl;
    std::cout << "[AIMBOT] - Max snap distance: " << Cheat::Config::Aimbot::maxAimSnapDistance << " degrees" << std::endl;
    
    // Apply reaction time delay
    static DWORD lastReactionTime = 0;
    DWORD currentTime = GetTickCount();
    if (currentTime - lastReactionTime < (Cheat::Config::Aimbot::reactionTime * 1000)) {
        std::cout << "[AIMBOT] Still in reaction period - delaying aim" << std::endl;
        return; // Still in reaction period
    }
    
    // Determine aiming speed based on distance
    float aimSpeed = Cheat::Config::Aimbot::maxTurnSpeed;

    // For large distances, use instant snap (simulating mouse flick)
    if (angularDistance > Cheat::Config::Aimbot::maxAimSnapDistance) {
        std::cout << "[AIMBOT] Large angular distance detected - using instant snap" << std::endl;
        playerController->SetControlRotation(targetRotation);
        lastReactionTime = currentTime;
        return;
    }
    
    // Add slight randomness to movement speed for human-like behavior
    float randomFactor = 0.8f + (rand() % 40) / 100.0f; // 0.8 to 1.2
    aimSpeed *= randomFactor;
    
    std::cout << "[AIMBOT] - Base aim speed: " << Cheat::Config::Aimbot::maxTurnSpeed << " deg/s" << std::endl;
    std::cout << "[AIMBOT] - Random factor: " << std::fixed << std::setprecision(2) << randomFactor << std::endl;
    std::cout << "[AIMBOT] - Final aim speed: " << std::fixed << std::setprecision(1) << aimSpeed << " deg/s" << std::endl;
    
    // Apply smooth rotation using UKismetMathLibrary
    SDK::FRotator smoothedRotation = Math::LerpRotation(
        currentRotation, 
        targetRotation, 
        deltaTime, 
        aimSpeed
    );
    
    playerController->SetControlRotation(smoothedRotation);
    std::cout << "[AIMBOT] Humanized rotation applied" << std::endl;
}

bool AimbotController::ValidateGameState(SDK::UWorld* world, 
                                       SDK::APlayerController* playerController,
                                       SDK::ARPlayerPawn* playerPawn) {
    if (!world || !world->PersistentLevel) {
        std::cout << "[AIMBOT] Validation failed: World or PersistentLevel is null" << std::endl;
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
    
    // Check if player is alive using the GetIsAlive method
    if (!playerPawn->GetIsAlive()) {
        std::cout << "[AIMBOT] Validation failed: Player is not alive" << std::endl;
        return false;
    }
    
    return true;
}