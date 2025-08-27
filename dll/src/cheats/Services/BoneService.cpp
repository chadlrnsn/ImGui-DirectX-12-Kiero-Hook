#include "BoneService.h"
#include "../Analysis/BoneAnalyzer.h"

namespace Cheat { namespace Services {

void BoneService::Initialize() {
    BoneAnalyzer::Initialize();
}

void BoneService::Shutdown() {
    // No-op for now
}

void BoneService::DumpUniqueEnemyBones(SDK::UWorld* world) {
    BoneAnalyzer::DumpUniqueEnemyBones(world);
}

void BoneService::DisplayBoneDatabase() {
    BoneAnalyzer::DisplayBoneDatabase();
}

void BoneService::DumpPlayerBones(SDK::ARPlayerPawn* playerPawn) {
    BoneAnalyzer::DumpBones(playerPawn->GetSkeletalMeshComponent());
}

int BoneService::GetTargetBoneIndex(const std::string& enemyKey) {
    return BoneAnalyzer::GetTargetBoneIndex(enemyKey);
}

std::string BoneService::GetTargetBoneName(const std::string& enemyKey) {
    return BoneAnalyzer::GetTargetBoneName(enemyKey);
}

std::string BoneService::BuildEnemyKey(SDK::AREnemyPawnBase* enemy, SDK::USkeletalMeshComponent* mesh) {
    return BoneAnalyzer::BuildEnemyKey(enemy, mesh);
}

bool BoneService::AnalyzeAndAddEnemy(SDK::AREnemyPawnBase* enemy, SDK::USkeletalMeshComponent* mesh) {
    return BoneAnalyzer::AnalyzeAndAddEnemy(enemy, mesh);
}

} } // namespace Cheat::Services

