#include "BoneAnalyzer.h"
#include <iostream>
#include <algorithm>
#include <filesystem>
#include <sstream>
#include <cctype>
#include <ctime>
#include <Windows.h>

namespace BoneAnalyzer {
    // Global storage for tracking dumped enemy types
    std::unordered_set<std::string> g_dumpedEnemyNames;
    
    // Global map for storing bone data per enemy type
    std::unordered_map<std::string, EnemyBoneData> g_enemyBoneMap;
    
    // File path for storing bone data
    std::string g_boneDataFilePath;

    // Initialize the bone analyzer (in-memory mode; no file IO)
    void Initialize() {
        g_dumpedEnemyNames.clear();
        g_enemyBoneMap.clear();
        std::cout << "[BONE ANALYZER] Initialized (in-memory cache)" << std::endl;
    }

    // Helper function to get a stable class name from an object by parsing GetFullName() and removing instance IDs
    std::string GetUObjectClassName(SDK::UObject* obj)
    {
        if (!obj) return std::string();
        std::string full = obj->GetFullName();
        size_t sp = full.find(' ');
        if (sp != std::string::npos) {
            std::string className = full.substr(0, sp);
            
            // Remove instance ID suffixes like "_2147475704" to get base class name
            size_t lastUnderscore = className.find_last_of('_');
            if (lastUnderscore != std::string::npos) {
                std::string potentialId = className.substr(lastUnderscore + 1);
                // Check if it's all digits (instance ID)
                bool isAllDigits = !potentialId.empty() && 
                    std::all_of(potentialId.begin(), potentialId.end(), ::isdigit);
                if (isAllDigits) {
                    className = className.substr(0, lastUnderscore);
                }
            }
            return className;
        }
        return full; // fallback
    }

    // Helper function to build a unique enemy key from an enemy pawn and its skeletal mesh
    std::string BuildEnemyKey(SDK::AREnemyPawnBase* enemy, SDK::USkeletalMeshComponent* mesh) {
        if (!enemy || !mesh) {
            return std::string();
        }
        
        // Build enemy key for bone database lookup
        std::string pawnBaseClassName = GetUObjectClassName(enemy);
        std::string animBPBaseClassName;
        if (auto animInstance = mesh->GetAnimInstance()) {
            animBPBaseClassName = GetUObjectClassName(animInstance);
        }
        
        return pawnBaseClassName + "|" + animBPBaseClassName;
    }

    // Function to find the best target bone from a list of bones with prioritization
    // Priority: Head,Body,Core,Torso,Spine_01,Spine; fallback to Root (lowest priority)
    EnemyBoneData AnalyzeBones(SDK::USkeletalMeshComponent* mesh) {
        EnemyBoneData boneData;

        if (!mesh) {
            std::cout << "[BONE ANALYZER] ERROR: SkeletalMeshComponent is null!" << std::endl;
            return boneData;
        }

        int boneCount = mesh->GetNumBones();
        std::cout << "[BONE ANALYZER] Analyzing " << boneCount << " bones..." << std::endl;

        // Priority list for target bones (case insensitive matching)
        std::vector<std::string> priorityBones = {"Head", "Body", "Core", "Torso", "Spine_01", "Spine"};
        const std::string rootName = "Root"; // fallback lowest priority

        // Collect all bones
        for (int i = 0; i < boneCount; i++) {
            std::string boneName = mesh->GetBoneName(i).ToString();
            boneData.allBones.emplace_back(i, boneName);
        }

        // Helper function to convert string to lowercase for case-insensitive comparison
        auto toLower = [](std::string str) {
            std::transform(str.begin(), str.end(), str.begin(), ::tolower);
            return str;
        };

        auto equalsIgnoreCase = [&](const std::string& a, const std::string& b){
            return toLower(a) == toLower(b);
        };

        // Find target bone using priority system (case insensitive)
        for (const std::string& priorityBone : priorityBones) {
            for (const auto& [boneIndex, boneName] : boneData.allBones) {
                if (equalsIgnoreCase(boneName, priorityBone)) {
                    boneData.targetBoneIndex = boneIndex;
                    boneData.targetBoneName = boneName; // Keep original case for display
                    std::cout << "[BONE ANALYZER] Selected priority bone: " << boneName << " (index " << boneIndex << ")" << std::endl;
                    return boneData;
                }
            }
        }

        // Try Root as the lowest-priority fallback
        for (const auto& [boneIndex, boneName] : boneData.allBones) {
            if (equalsIgnoreCase(boneName, rootName)) {
                boneData.targetBoneIndex = boneIndex;
                boneData.targetBoneName = boneName;
                std::cout << "[BONE ANALYZER] No priority bones found, using Root: " << boneName
                          << " (index " << boneIndex << ")" << std::endl;
                return boneData;
            }
        }

        // If no priority or Root found, use the first bone as absolute last resort
        if (!boneData.allBones.empty()) {
            boneData.targetBoneIndex = boneData.allBones[0].first;
            boneData.targetBoneName = boneData.allBones[0].second;
            std::cout << "[BONE ANALYZER] No priority or Root; using first bone: " << boneData.targetBoneName
                      << " (index " << boneData.targetBoneIndex << ")" << std::endl;
        } else {
            std::cout << "[BONE ANALYZER] WARNING: No bones found in mesh!" << std::endl;
        }

        return boneData;
    }

    // Function to dump bones for a single skeletal mesh component
    void DumpBones(SDK::USkeletalMeshComponent* mesh)
    {
		
        if (!mesh) {
            std::cout << "ERROR: SkeletalMeshComponent is null!" << std::endl;
            return;
        }

        int boneCount = mesh->GetNumBones();
        std::cout << "  Total bones: " << boneCount << std::endl;
        
        for (int b = 0; b < boneCount; b++) {
            std::string boneName = mesh->GetBoneName(b).ToString();
            std::cout << "    Bone [" << b << "]: " << boneName << std::endl;
        }
    }

    // Main function to iterate enemies and dump bones only once per unique enemy "type"
    void DumpUniqueEnemyBones(SDK::UWorld* World)
    {
        if (!World || !World->PersistentLevel) {
            std::cout << "ERROR: World or PersistentLevel not available!" << '\n';
            return;
        }

        auto& currentActors = World->PersistentLevel->Actors;
        std::cout << "\n=== F3 PRESSED - ENEMY BONES DUMP ===" << '\n';
        std::cout << "Total actors count: " << currentActors.Num() << std::endl;

        int enemiesSeen = 0;
        int newlyDumped = 0;

        if (currentActors.Num() > 0) {
            for (int i = 0; i < currentActors.Num(); ++i) {
                auto actor = currentActors[i];
                if (actor && actor->IsA(SDK::AREnemyPawnBase::StaticClass())) {
                    auto enemy = static_cast<SDK::AREnemyPawnBase*>(actor);

                    // Use the exposed SkeletalMesh property
                    SDK::USkeletalMeshComponent* mesh = enemy->SkeletalMesh;
                    if (!mesh) {
                        continue;
                    }

                    // Build a stable uniqueness key using the helper function
                    std::string uniqueKey = BuildEnemyKey(enemy, mesh);
                    if (uniqueKey.empty()) {
                        std::cout << "ERROR: Failed to build enemy key for " << enemy->GetName() << std::endl;
                        continue;
                    }

                    ++enemiesSeen;

                    if (g_dumpedEnemyNames.contains(uniqueKey)) {
                        std::cout << "Skipping already analyzed enemy type: " << uniqueKey << std::endl;
                        continue;
                    }

                    std::cout << "Analyzing bones for enemy type: " << uniqueKey << std::endl;
                    std::cout << "  (Instance: " << enemy->GetName() << ")" << std::endl;
                    
                    // Analyze bones and store the data
                    EnemyBoneData boneData = AnalyzeBones(mesh);
                    g_enemyBoneMap[uniqueKey] = boneData;
                    
                    // Also dump all bones for reference
                    DumpBones(mesh);
                    
                    g_dumpedEnemyNames.insert(uniqueKey);
                    ++newlyDumped;
                }
            }
        }

        // In-memory mode: no file persistence

        std::cout << "Enemy actors encountered: " << enemiesSeen << std::endl;
        std::cout << "New unique types analyzed this run: " << newlyDumped << std::endl;
        std::cout << "Previously analyzed types (skipped) this run: "
                  << (enemiesSeen - newlyDumped) << std::endl;
        std::cout << "Total enemy types in database: " << g_enemyBoneMap.size() << std::endl;
        std::cout << "=== END ENEMY BONES DUMP ===\n" << std::endl;
    }

    // Get the target bone index for a specific enemy type (for use by aimbot)
    int GetTargetBoneIndex(const std::string& enemyKey) {
        auto it = g_enemyBoneMap.find(enemyKey);
        if (it != g_enemyBoneMap.end()) {
            return it->second.targetBoneIndex;
        }
        return -1; // Not found
    }

    // Get the target bone name for a specific enemy type (for debugging)
    std::string GetTargetBoneName(const std::string& enemyKey) {
        auto it = g_enemyBoneMap.find(enemyKey);
        if (it != g_enemyBoneMap.end()) {
            return it->second.targetBoneName;
        }
        return "Unknown"; // Not found
    }

    // Automatically analyze and add a single enemy to the database
    bool AnalyzeAndAddEnemy(SDK::AREnemyPawnBase* enemy, SDK::USkeletalMeshComponent* mesh) {
        if (!enemy || !mesh) {
            std::cout << "[BONE ANALYZER] AnalyzeAndAddEnemy: Invalid parameters (enemy or mesh is null)" << std::endl;
            return false;
        }

        // Build enemy key
        std::string enemyKey = BuildEnemyKey(enemy, mesh);
        if (enemyKey.empty()) {
            std::cout << "[BONE ANALYZER] AnalyzeAndAddEnemy: Failed to build enemy key" << std::endl;
            return false;
        }

        // Check if already exists (shouldn't happen, but safety check)
        if (g_enemyBoneMap.find(enemyKey) != g_enemyBoneMap.end()) {
            std::cout << "[BONE ANALYZER] AnalyzeAndAddEnemy: Enemy already exists in database: " << enemyKey << std::endl;
            return true; // Already exists, consider it success
        }

        std::cout << "[BONE ANALYZER] Auto-analyzing new enemy type: " << enemyKey << std::endl;
        std::cout << "[BONE ANALYZER] - Instance: " << enemy->GetName() << std::endl;

        // Analyze bones and store the data
        EnemyBoneData boneData = AnalyzeBones(mesh);
        if (boneData.targetBoneIndex == -1) {
            std::cout << "[BONE ANALYZER] AnalyzeAndAddEnemy: Failed to find target bone for " << enemyKey << std::endl;
            return false;
        }

        // Add to database
        g_enemyBoneMap[enemyKey] = boneData;
        g_dumpedEnemyNames.insert(enemyKey);

        std::cout << "[BONE ANALYZER] Successfully added new enemy type: " << enemyKey << std::endl;
        std::cout << "[BONE ANALYZER] - Target bone: " << boneData.targetBoneName << " (index " << boneData.targetBoneIndex << ")" << std::endl;

        // In-memory mode: no file persistence

        return true;
    }

    // Display current bone database status (for debugging)
    void DisplayBoneDatabase() {
        std::cout << "\n=== BONE DATABASE STATUS ===" << std::endl;
        std::cout << "Total enemy types analyzed: " << g_enemyBoneMap.size() << std::endl;

        if (!g_enemyBoneMap.empty()) {
            std::cout << "\nEnemy types and their target bones:" << std::endl;
            for (const auto& [enemyKey, boneData] : g_enemyBoneMap) {
                std::cout << "  " << enemyKey << " -> " << boneData.targetBoneName 
                          << " (index " << boneData.targetBoneIndex << ")" << std::endl;
            }
        } else {
            std::cout << "No enemy bone data available. Use F3 to scan for enemies." << std::endl;
        }
        std::cout << "=== END BONE DATABASE STATUS ===\n" << std::endl;
    }
}