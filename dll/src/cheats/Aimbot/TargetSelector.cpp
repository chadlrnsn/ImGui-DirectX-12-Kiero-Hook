#include "TargetSelector.h"
#include "MathUtils.h"
#include "../Analysis/BoneAnalyzer.h"
#include <iomanip>

TargetInfo TargetSelector::SelectBestTarget(SDK::UWorld* world, 
                                           SDK::APlayerController* playerController,
                                           SDK::ARPlayerPawn* playerPawn) {
    TargetInfo bestTarget;
    float bestScore = FLT_MAX;
    
    std::cout << "[TARGET_SELECTOR] Starting target selection..." << std::endl;
    
    if (!world || !world->PersistentLevel || !playerController || !playerPawn) {
        std::cout << "[TARGET_SELECTOR] ERROR: Invalid parameters" << std::endl;
        std::cout << "[TARGET_SELECTOR] - World: " << (world ? "valid" : "null") << std::endl;
        std::cout << "[TARGET_SELECTOR] - PersistentLevel: " << (world && world->PersistentLevel ? "valid" : "null") << std::endl;
        std::cout << "[TARGET_SELECTOR] - PlayerController: " << (playerController ? "valid" : "null") << std::endl;
        std::cout << "[TARGET_SELECTOR] - PlayerPawn: " << (playerPawn ? "valid" : "null") << std::endl;
        return bestTarget;
    }
    
    SDK::FVector playerLocation = playerPawn->K2_GetActorLocation();
    SDK::FRotator controlRotation = playerController->K2_GetActorRotation();
    
    std::cout << "[TARGET_SELECTOR] Player info:" << std::endl;
    std::cout << "[TARGET_SELECTOR] - Location: (" << std::fixed << std::setprecision(1) 
              << playerLocation.X << ", " << playerLocation.Y << ", " << playerLocation.Z << ")" << std::endl;
    std::cout << "[TARGET_SELECTOR] - Rotation: (" << std::fixed << std::setprecision(1) 
              << controlRotation.Pitch << ", " << controlRotation.Yaw << ", " << controlRotation.Roll << ")" << std::endl;
    
    // Get all actors in the level
    auto& actors = world->PersistentLevel->Actors;
    
    std::cout << "[TARGET_SELECTOR] Total actors in level: " << actors.Num() << std::endl;
    std::cout << "[TARGET_SELECTOR] Filtering criteria:" << std::endl;
    std::cout << "[TARGET_SELECTOR] - Max distance: " << g_AimbotConfig.maxDistance << " units" << std::endl;
    std::cout << "[TARGET_SELECTOR] - FOV radius: " << g_AimbotConfig.fovRadius << " degrees" << std::endl;
    std::cout << "[TARGET_SELECTOR] - Visibility check: " << (g_AimbotConfig.visibilityCheck ? "enabled" : "disabled") << std::endl;
    
    int validActorsCount = 0;
    int distanceFilteredCount = 0;
    int fovFilteredCount = 0;
    int visibilityFilteredCount = 0;
    int finalValidCount = 0;
    
    for (int i = 0; i < actors.Num(); i++) {
        auto actor = actors[i];
        if (!IsValidTarget(actor)) continue;
        
        validActorsCount++;
        std::cout << "[TARGET_SELECTOR] Valid actor #" << validActorsCount << ": " << actor->GetName() << std::endl;
        
        TargetInfo targetInfo;
        targetInfo.actor = actor;
        targetInfo.position = actor->K2_GetActorLocation();
        
        // Try to get bone-based aim point first
        targetInfo.aimPoint = GetBoneBasedAimPoint(actor, targetInfo.targetBoneIndex, targetInfo.targetBoneName);
        if (targetInfo.targetBoneIndex != -1) {
            targetInfo.hasBoneTarget = true;
            std::cout << "[TARGET_SELECTOR] - Using bone-based targeting: " << targetInfo.targetBoneName 
                      << " (index " << targetInfo.targetBoneIndex << ")" << std::endl;
        } else {
            targetInfo.hasBoneTarget = false;
            targetInfo.aimPoint = GetTargetAimPoint(actor);
            std::cout << "[TARGET_SELECTOR] - Using fallback aim point (no bone data)" << std::endl;
        }
        
        // Use built-in UE distance function
        targetInfo.distance = playerLocation.GetDistanceTo(targetInfo.position);
        
        std::cout << "[TARGET_SELECTOR] - Position: (" << std::fixed << std::setprecision(1) 
                  << targetInfo.position.X << ", " << targetInfo.position.Y << ", " << targetInfo.position.Z << ")" << std::endl;
        std::cout << "[TARGET_SELECTOR] - Aim point: (" << std::fixed << std::setprecision(1) 
                  << targetInfo.aimPoint.X << ", " << targetInfo.aimPoint.Y << ", " << targetInfo.aimPoint.Z << ")" << std::endl;
        std::cout << "[TARGET_SELECTOR] - Distance: " << std::fixed << std::setprecision(1) << targetInfo.distance << " units" << std::endl;
        
        // Skip if too far
        if (targetInfo.distance > g_AimbotConfig.maxDistance) {
            std::cout << "[TARGET_SELECTOR] - FILTERED OUT: Too far (>" << g_AimbotConfig.maxDistance << ")" << std::endl;
            distanceFilteredCount++;
            continue;
        }
        
        // // Use screen-based FOV check for more accurate targeting
        // if (!Math::IsWithinScreenFOV(targetInfo.aimPoint, playerLocation, playerController, g_AimbotConfig.fovRadius)) {
        //     std::cout << "[TARGET_SELECTOR] - FILTERED OUT: Outside FOV" << std::endl;
        //     fovFilteredCount++;
        //     continue;
        // }
        
        // Calculate FOV distance using screen projection for more accurate prioritization
        SDK::FVector2D targetScreenPos;
        if (Math::WorldToScreen(targetInfo.aimPoint, &targetScreenPos, playerController)) {
            SDK::FVector2D screenCenter = Math::GetScreenCenter(playerController);
            targetInfo.fovDistance = targetScreenPos.GetDistanceTo(screenCenter);
            
            std::cout << "[TARGET_SELECTOR] - Screen position: (" << std::fixed << std::setprecision(1) 
                      << targetScreenPos.X << ", " << targetScreenPos.Y << ")" << std::endl;
            std::cout << "[TARGET_SELECTOR] - Screen center: (" << std::fixed << std::setprecision(1) 
                      << screenCenter.X << ", " << screenCenter.Y << ")" << std::endl;
            std::cout << "[TARGET_SELECTOR] - FOV distance: " << std::fixed << std::setprecision(1) 
                      << targetInfo.fovDistance << " pixels" << std::endl;
        } else {
            std::cout << "[TARGET_SELECTOR] - FILTERED OUT: Cannot project to screen" << std::endl;
            continue; // Skip if we can't project to screen
        }
        
        // Visibility check
        if (g_AimbotConfig.visibilityCheck) {
            targetInfo.isVisible = IsTargetVisible(world, playerLocation, targetInfo.aimPoint, actor, playerPawn);
            std::cout << "[TARGET_SELECTOR] - Visibility check: " << (targetInfo.isVisible ? "VISIBLE" : "BLOCKED") << std::endl;
            if (!targetInfo.isVisible) {
                std::cout << "[TARGET_SELECTOR] - FILTERED OUT: Not visible" << std::endl;
                visibilityFilteredCount++;
                continue;
            }
        } else {
            targetInfo.isVisible = true;
            std::cout << "[TARGET_SELECTOR] - Visibility check: SKIPPED" << std::endl;
        }
        
        // Calculate priority score (lower is better)
        float score = CalculateTargetPriority(targetInfo, playerLocation);
        std::cout << "[TARGET_SELECTOR] - Priority score: " << std::fixed << std::setprecision(2) << score << std::endl;
        
        if (score < bestScore) {
            std::cout << "[TARGET_SELECTOR] - NEW BEST TARGET! (previous best score: " 
                      << std::fixed << std::setprecision(2) << bestScore << ")" << std::endl;
            bestScore = score;
            bestTarget = targetInfo;
        } else {
            std::cout << "[TARGET_SELECTOR] - Not better than current best (score: " 
                      << std::fixed << std::setprecision(2) << bestScore << ")" << std::endl;
        }
        
        finalValidCount++;
    }
    
    std::cout << "[TARGET_SELECTOR] Selection summary:" << std::endl;
    std::cout << "[TARGET_SELECTOR] - Total actors: " << actors.Num() << std::endl;
    std::cout << "[TARGET_SELECTOR] - Valid enemy actors: " << validActorsCount << std::endl;
    std::cout << "[TARGET_SELECTOR] - Filtered by distance: " << distanceFilteredCount << std::endl;
    std::cout << "[TARGET_SELECTOR] - Filtered by FOV: " << fovFilteredCount << std::endl;
    std::cout << "[TARGET_SELECTOR] - Filtered by visibility: " << visibilityFilteredCount << std::endl;
    std::cout << "[TARGET_SELECTOR] - Final valid targets: " << finalValidCount << std::endl;
    
    if (bestTarget.actor) {
        std::cout << "[TARGET_SELECTOR] BEST TARGET SELECTED: " << bestTarget.actor->GetName() 
                  << " (score: " << std::fixed << std::setprecision(2) << bestScore << ")" << std::endl;
        if (bestTarget.hasBoneTarget) {
            std::cout << "[TARGET_SELECTOR] - Target bone: " << bestTarget.targetBoneName 
                      << " (index " << bestTarget.targetBoneIndex << ")" << std::endl;
        }
    } else {
        std::cout << "[TARGET_SELECTOR] NO TARGET SELECTED" << std::endl;
    }
    
    return bestTarget;
}

bool TargetSelector::IsValidTarget(SDK::AActor* actor) {
    if (!actor) {
        std::cout << "[TARGET_SELECTOR] Invalid target: actor is null" << std::endl;
        return false;
    }
    
    // Check if it's an NPC/Enemy pawn using the correct class
    if (!actor->IsA(SDK::AREnemyPawnBase::StaticClass())) {
        // Don't log this as it would spam too much - most actors aren't enemies
        return false;
    }
    
    std::cout << "[TARGET_SELECTOR] Found enemy actor: " << actor->GetName() << std::endl;
    
    // Cast to NPC pawn to check health
    auto npcPawn = static_cast<SDK::AREnemyPawnBase*>(actor);
    if (!npcPawn) {
        std::cout << "[TARGET_SELECTOR] Invalid target: failed to cast to AREnemyPawnBase" << std::endl;
        return false;
    }
    
    // Check if it has a health component and is alive
    auto healthComponent = npcPawn->GetHealthComponent();
    if (!healthComponent) {
        std::cout << "[TARGET_SELECTOR] Invalid target: no health component" << std::endl;
        return false;
    }
    
    // Use GetIsAlive method from ARPawnBase (inherited by AREnemyPawnBase)
    if (!npcPawn->GetIsAlive()) {
        std::cout << "[TARGET_SELECTOR] Invalid target: not alive" << std::endl;
        return false;
    }
    
    std::cout << "[TARGET_SELECTOR] Valid target confirmed: " << actor->GetName() << std::endl;
    return true;
}

bool TargetSelector::IsTargetVisible(SDK::UWorld* world, 
                                    const SDK::FVector& from, 
                                    const SDK::FVector& to,
                                    SDK::AActor* targetActor,
                                    SDK::AActor* playerActor) {
    if (!world) {
        std::cout << "[TARGET_SELECTOR] Visibility check failed: world is null" << std::endl;
        return false;
    }
    
    std::cout << "[TARGET_SELECTOR] Performing line trace visibility check..." << std::endl;
    std::cout << "[TARGET_SELECTOR] - From: (" << std::fixed << std::setprecision(1) 
              << from.X << ", " << from.Y << ", " << from.Z << ")" << std::endl;
    std::cout << "[TARGET_SELECTOR] - To: (" << std::fixed << std::setprecision(1) 
              << to.X << ", " << to.Y << ", " << to.Z << ")" << std::endl;
    
    // Setup trace parameters
    const SDK::ETraceTypeQuery traceChannel = SDK::ETraceTypeQuery::TraceTypeQuery1;
    SDK::TArray<SDK::AActor*> actorsToIgnore;
    
    if (playerActor) {
        actorsToIgnore.Add(playerActor);
        std::cout << "[TARGET_SELECTOR] - Ignoring player actor: " << playerActor->GetName() << std::endl;
    }
    
    // Perform line trace
    SDK::FHitResult hitResult;
    memset(&hitResult, 0, sizeof(SDK::FHitResult));
    
    bool hit = SDK::UKismetSystemLibrary::LineTraceSingle(
        world,
        from,
        to,
        traceChannel,
        false, // bTraceComplex
        actorsToIgnore,
        SDK::EDrawDebugTrace::None,
        &hitResult,
        true, // bIgnoreSelf
        SDK::FLinearColor(), // TraceColor
        SDK::FLinearColor(), // TraceHitColor
        0.0f // DrawTime
    );
    
    if (!hit) {
        std::cout << "[TARGET_SELECTOR] - Line trace result: NO HIT - target is visible" << std::endl;
        return true;
    }
    
    std::cout << "[TARGET_SELECTOR] - Line trace result: HIT - something is blocking" << std::endl;
    if (hitResult.HitObjectHandle.ReferenceObject.Get()) {
        std::cout << "[TARGET_SELECTOR] - Blocking actor: " << hitResult.HitObjectHandle.ReferenceObject.Get()->GetName() << std::endl;
        
        
    } else {
        std::cout << "[TARGET_SELECTOR] - No blocking actor info available" << std::endl;
    }
    
    // Simplified visibility check - if we hit something else, assume it's blocking
    return false;
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
    
    std::cout << "[TARGET_SELECTOR] Looking up bone data for enemy: " << enemyKey << std::endl;
    
    // Get target bone from database
    int targetBoneIndex = BoneAnalyzer::GetTargetBoneIndex(enemyKey);
    std::string targetBoneName = BoneAnalyzer::GetTargetBoneName(enemyKey);
    
    if (targetBoneIndex == -1) {
        std::cout << "[TARGET_SELECTOR] No bone data found for enemy type: " << enemyKey << std::endl;
        std::cout << "[TARGET_SELECTOR] Attempting to auto-analyze and add to database..." << std::endl;
        
        // Try to automatically analyze and add this enemy to the database
        if (BoneAnalyzer::AnalyzeAndAddEnemy(enemy, mesh)) {
            std::cout << "[TARGET_SELECTOR] Successfully auto-analyzed enemy, retrying bone lookup..." << std::endl;
            
            // Retry the lookup after adding to database
            targetBoneIndex = BoneAnalyzer::GetTargetBoneIndex(enemyKey);
            targetBoneName = BoneAnalyzer::GetTargetBoneName(enemyKey);
            
            if (targetBoneIndex == -1) {
                std::cout << "[TARGET_SELECTOR] ERROR: Still no bone data after auto-analysis!" << std::endl;
                return SDK::FVector();
            }
        } else {
            std::cout << "[TARGET_SELECTOR] Failed to auto-analyze enemy, falling back to generic targeting" << std::endl;
            return SDK::FVector();
        }
    }
    
    // Get the bone name from the mesh
    SDK::FName boneName = mesh->GetBoneName(targetBoneIndex);
    if (boneName.ToString().empty()) {
        std::cout << "[TARGET_SELECTOR] ERROR: Invalid bone index " << targetBoneIndex << std::endl;
        return SDK::FVector();
    }
    
    // Get the bone location using GetSocketLocation
    SDK::FVector boneLocation = mesh->GetSocketLocation(boneName);
    
    std::cout << "[TARGET_SELECTOR] Bone location: (" << std::fixed << std::setprecision(1) 
              << boneLocation.X << ", " << boneLocation.Y << ", " << boneLocation.Z << ")" << std::endl;
    
    // Set output parameters
    outBoneIndex = targetBoneIndex;
    outBoneName = targetBoneName;
    
    return boneLocation;
}

SDK::FVector TargetSelector::GetTargetAimPoint(SDK::AActor* targetActor) {
    if (!targetActor) {
        std::cout << "[TARGET_SELECTOR] GetTargetAimPoint: actor is null" << std::endl;
        return SDK::FVector();
    }
    
    SDK::FVector actorLocation = targetActor->K2_GetActorLocation();
    
    // Try to aim for the head/upper body by adding some height offset
    // This is a simple implementation - could be improved with bone targeting
    actorLocation.Z += 10.0f; // Approximate head height offset
    
    std::cout << "[TARGET_SELECTOR] Fallback aim point calculation:" << std::endl;
    std::cout << "[TARGET_SELECTOR] - Base location: (" << std::fixed << std::setprecision(1) 
              << (actorLocation.Z - 10.0f) << ")" << std::endl;
    std::cout << "[TARGET_SELECTOR] - Head offset: +10.0" << std::endl;
    std::cout << "[TARGET_SELECTOR] - Final aim point: (" << std::fixed << std::setprecision(1) 
              << actorLocation.X << ", " << actorLocation.Y << ", " << actorLocation.Z << ")" << std::endl;
    
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
    float normalizedDistance = target.distance / g_AimbotConfig.maxDistance;
    
    float finalScore = target.fovDistance * fovWeight + normalizedDistance * distanceWeight + boneTargetBonus;
    
    std::cout << "[TARGET_SELECTOR] Priority calculation:" << std::endl;
    std::cout << "[TARGET_SELECTOR] - FOV distance: " << std::fixed << std::setprecision(1) << target.fovDistance 
              << " * " << fovWeight << " = " << (target.fovDistance * fovWeight) << std::endl;
    std::cout << "[TARGET_SELECTOR] - Normalized distance: " << std::fixed << std::setprecision(3) << normalizedDistance 
              << " * " << distanceWeight << " = " << (normalizedDistance * distanceWeight) << std::endl;
    std::cout << "[TARGET_SELECTOR] - Bone target bonus: " << std::fixed << std::setprecision(1) << boneTargetBonus << std::endl;
    std::cout << "[TARGET_SELECTOR] - Final score: " << std::fixed << std::setprecision(2) << finalScore 
              << " (lower is better)" << std::endl;
    
    // Combine FOV distance, physical distance, and bone targeting bonus
    return finalScore;
}