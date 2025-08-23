#pragma once

#include "../SDK/Engine_classes.hpp"
#include "../SDK/RGame_classes.hpp"
#include "../Analysis/BoneAnalyzer.h"
#include "../Core/Config.h"

class TargetSelector {
public:
    static TargetInfo SelectBestTarget(SDK::UWorld* world, 
                                     SDK::APlayerController* playerController,
                                     SDK::ARPlayerPawn* playerPawn);
    
private:
    static bool IsValidTarget(SDK::AActor* actor);
    static bool IsTargetVisible(SDK::UWorld* world, 
                               const SDK::FVector& from, 
                               const SDK::FVector& to,
                               SDK::AActor* targetActor,
                               SDK::AActor* playerActor);
    
    static SDK::FVector GetTargetAimPoint(SDK::AActor* targetActor);
    static SDK::FVector GetBoneBasedAimPoint(SDK::AActor* targetActor, int& outBoneIndex, std::string& outBoneName);
    static float CalculateTargetPriority(const TargetInfo& target, 
                                        const SDK::FVector& playerPos);
};