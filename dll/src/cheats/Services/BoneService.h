#pragma once

#include "../SDK/Engine_classes.hpp"
#include "../SDK/RGame_classes.hpp"
#include <string>

namespace Cheat { namespace Services {

class BoneService {
public:
    // Lifecycle
    static void Initialize();
    static void Shutdown();

    // Analysis actions
    static void DumpUniqueEnemyBones(SDK::UWorld* world);
    static void DisplayBoneDatabase();

    // Queries
    static int GetTargetBoneIndex(const std::string& enemyKey);
    static std::string GetTargetBoneName(const std::string& enemyKey);

    // Utilities used by targeting
    static std::string BuildEnemyKey(SDK::AREnemyPawnBase* enemy, SDK::USkeletalMeshComponent* mesh);
    static bool AnalyzeAndAddEnemy(SDK::AREnemyPawnBase* enemy, SDK::USkeletalMeshComponent* mesh);
};

} } // namespace Cheat::Services

