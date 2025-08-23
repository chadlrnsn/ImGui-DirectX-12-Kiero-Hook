#pragma once

#include "../SDK/Engine_classes.hpp"
#include "../SDK/RGame_classes.hpp"
#include "../Analysis/BoneAnalyzer.h"

struct TargetInfo {
    SDK::AActor* actor = nullptr;
    SDK::FVector position;
    float distance = 0.0f;
    float fovDistance = 0.0f;
    bool isVisible = false;
    SDK::FVector aimPoint;
    
    // Bone targeting information
    int targetBoneIndex = -1;
    std::string targetBoneName;
    bool hasBoneTarget = false;
};

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