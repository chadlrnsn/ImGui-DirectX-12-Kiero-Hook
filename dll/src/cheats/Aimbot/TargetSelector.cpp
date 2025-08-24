#include "TargetSelector.h"
#include "MathUtils.h"
#include "../Analysis/BoneAnalyzer.h"
#include "../Core/Config.h"
#include <dev/validity.h>
#include <iomanip>
#include <iostream>
#include <algorithm>
#include <mutex>

// External mutex for thread-safe access to target list
extern std::mutex list_mutex;

TargetInfo TargetSelector::SelectBestTarget(SDK::UWorld* world,
                                           SDK::APlayerController* playerController,
                                           SDK::ARPlayerPawn* playerPawn) {
    TargetInfo bestTarget;
    float bestScore = FLT_MAX;
    
    if (!world || !world->PersistentLevel || !playerController || !playerPawn) {
        return bestTarget;
    }

    SDK::FVector playerLocation = playerPawn->K2_GetActorLocation();
    SDK::FRotator controlRotation = playerController->K2_GetActorRotation();
    
    // Use centralized target list populated by MainLoop
    std::vector<SDK::AActor*> currentTargets;
    {
        // Thread-safe access to the centralized target list
        std::lock_guard<std::mutex> lock(list_mutex);
        currentTargets = Cheat::Config::GameState::g_TargetsList;
    }



    int validActorsCount = 0;
    int distanceFilteredCount = 0;
    int fovFilteredCount = 0;
    int visibilityFilteredCount = 0;
    int finalValidCount = 0;

    for (auto* actor : currentTargets) {
        // Validate actor using the existing validity checker
        if (!actor || Validity::IsBadPoint(actor)) continue;
        if (!IsValidTarget(actor)) continue;
        
        validActorsCount++;
        
        TargetInfo targetInfo;
        targetInfo.actor = actor;
        targetInfo.position = actor->K2_GetActorLocation();
        
        // Try to get bone-based aim point first
        targetInfo.aimPoint = GetBoneBasedAimPoint(actor, targetInfo.targetBoneIndex, targetInfo.targetBoneName);
        if (targetInfo.targetBoneIndex != -1) {
            targetInfo.hasBoneTarget = true;
        } else {
            targetInfo.hasBoneTarget = false;
            targetInfo.aimPoint = GetTargetAimPoint(actor);
        }
        
        // Use built-in UE distance function
        targetInfo.distance = playerLocation.GetDistanceTo(targetInfo.position);

        // Skip if too far
        if (targetInfo.distance > Cheat::Config::Aimbot::maxDistance) {
            distanceFilteredCount++;
            continue;
        }
        
        // // Use screen-based FOV check for more accurate targeting
        // if (!Math::IsWithinScreenFOV(targetInfo.aimPoint, playerLocation, playerController, Cheat::Config::Aimbot::fovRadius)) {
        //     std::cout << "[TARGET_SELECTOR] - FILTERED OUT: Outside FOV" << std::endl;
        //     fovFilteredCount++;
        //     continue;
        // }
        
        // Calculate FOV distance using screen projection for more accurate prioritization
        SDK::FVector2D targetScreenPos;
        if (Math::WorldToScreen(targetInfo.aimPoint, &targetScreenPos, playerController)) {
            SDK::FVector2D screenCenter = Math::GetScreenCenter(playerController);
            targetInfo.fovDistance = targetScreenPos.GetDistanceTo(screenCenter);
        } else {
            continue; // Skip if we can't project to screen
        }
        
        // Visibility check using LineOfSightTo
        if (Cheat::Config::Aimbot::visibilityCheck) {
            // Get camera position for more accurate LOS check
            SDK::FVector cameraLocation;
            if (playerController->PlayerCameraManager && playerController->PlayerCameraManager->CameraCachePrivate.POV.Location.X != 0.0f) {
                cameraLocation = playerController->PlayerCameraManager->CameraCachePrivate.POV.Location;
            } else {
                // Fallback to player location if camera not available
                cameraLocation = playerLocation;
            }

            // Cast target actor to pawn for LineOfSightTo
            SDK::APawn* targetPawn = static_cast<SDK::APawn*>(actor);
            if (targetPawn) {
                targetInfo.isVisible = playerController->LineOfSightTo(targetPawn, cameraLocation, false);
            } else {
                targetInfo.isVisible = false; // Can't check LOS for non-pawn actors
            }

            if (!targetInfo.isVisible) {
                visibilityFilteredCount++;
                continue;
            }
        } else {
            targetInfo.isVisible = true;
        }
        
        // Calculate priority score (lower is better)
        float score = CalculateTargetPriority(targetInfo, playerLocation);

        if (score < bestScore) {
            bestScore = score;
            bestTarget = targetInfo;
        }
        
        finalValidCount++;
    }
    

    
    return bestTarget;
}

bool TargetSelector::IsValidTarget(SDK::AActor* actor) {
    if (!actor) {
        return false;
    }

    // Check if it's an NPC/Enemy pawn using the correct class
    if (!actor->IsA(SDK::AREnemyPawnBase::StaticClass())) {
        return false;
    }

    // Cast to NPC pawn to check health
    auto npcPawn = static_cast<SDK::AREnemyPawnBase*>(actor);
    if (!npcPawn) {
        return false;
    }

    // Check if it has a health component and is alive
    auto healthComponent = npcPawn->GetHealthComponent();
    if (!healthComponent) {
        return false;
    }

    // Use GetIsAlive method from ARPawnBase (inherited by AREnemyPawnBase)
    if (!npcPawn->GetIsAlive()) {
        return false;
    }

    return true;
}



SDK::FVector TargetSelector::GetBoneBasedAimPoint(SDK::AActor* targetActor, int& outBoneIndex, std::string& outBoneName) {
    outBoneIndex = -1;
    outBoneName = "none";
    
    if (!targetActor) {
        std::cout << "[TARGET_SELECTOR] GetBoneBasedAimPoint: actor is null" << std::endl;
        return SDK::FVector();
    }
    
    // Check if it's an enemy pawn
    auto enemy = static_cast<SDK::AREnemyPawnBase*>(targetActor);
    if (!enemy) {
        std::cout << "[TARGET_SELECTOR] GetBoneBasedAimPoint: not an enemy pawn" << std::endl;
        return SDK::FVector();
    }
    
    // Get the skeletal mesh component
    SDK::USkeletalMeshComponent* mesh = enemy->SkeletalMesh;
    if (!mesh) {
        std::cout << "[TARGET_SELECTOR] GetBoneBasedAimPoint: no skeletal mesh" << std::endl;
        return SDK::FVector();
    }
    
    // Build enemy key for bone database lookup using the helper function
    std::string enemyKey = BoneAnalyzer::BuildEnemyKey(enemy, mesh);
    if (enemyKey.empty()) {
        std::cout << "[TARGET_SELECTOR] GetBoneBasedAimPoint: failed to build enemy key" << std::endl;
        return SDK::FVector();
    }
    

    
    // Get target bone from database
    int targetBoneIndex = BoneAnalyzer::GetTargetBoneIndex(enemyKey);
    std::string targetBoneName = BoneAnalyzer::GetTargetBoneName(enemyKey);
    
    if (targetBoneIndex == -1) {
        // Try to automatically analyze and add this enemy to the database
        if (BoneAnalyzer::AnalyzeAndAddEnemy(enemy, mesh)) {
            // Retry the lookup after adding to database
            targetBoneIndex = BoneAnalyzer::GetTargetBoneIndex(enemyKey);
            targetBoneName = BoneAnalyzer::GetTargetBoneName(enemyKey);

            if (targetBoneIndex == -1) {
                return SDK::FVector();
            }
        } else {
            return SDK::FVector();
        }
    }
    
    // Get the bone name from the mesh
    SDK::FName boneName = mesh->GetBoneName(targetBoneIndex);
    if (boneName.ToString().empty()) {
        return SDK::FVector();
    }

    // Get the bone location using GetSocketLocation
    SDK::FVector boneLocation = mesh->GetSocketLocation(boneName);
    
    // Set output parameters
    outBoneIndex = targetBoneIndex;
    outBoneName = targetBoneName;
    
    return boneLocation;
}

SDK::FVector TargetSelector::GetTargetAimPoint(SDK::AActor* targetActor) {
    if (!targetActor) {
        return SDK::FVector();
    }

    SDK::FVector actorLocation = targetActor->K2_GetActorLocation();

    // Try to aim for the head/upper body by adding some height offset
    actorLocation.Z += 10.0f; // Approximate head height offset

    return actorLocation;
}

float TargetSelector::CalculateTargetPriority(const TargetInfo& target, const SDK::FVector& playerPos) {
    // Priority calculation: closer to crosshair = higher priority
    // We can also factor in distance, but FOV distance is more important for aimbot
    
    float fovWeight = 1.0f;
    float distanceWeight = 0.3f;
    
    // Bonus for having bone-based targeting
    float boneTargetBonus = target.hasBoneTarget ? -10.0f : 0.0f; // Negative score is better
    
    // Normalize distance (closer targets get lower scores)
    float normalizedDistance = target.distance / Cheat::Config::Aimbot::maxDistance;
    
    float finalScore = target.fovDistance * fovWeight + normalizedDistance * distanceWeight + boneTargetBonus;

    return finalScore;
}