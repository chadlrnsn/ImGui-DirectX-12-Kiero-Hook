#pragma once

#include <string>
#include <unordered_set>
#include <unordered_map>
#include <fstream>
#include <vector>
#include "../SDK/Engine_classes.hpp"
#include "../SDK/RGame_classes.hpp"

namespace BoneAnalyzer {
    // Structure to hold prioritized bone information for an enemy type
    struct EnemyBoneData {
        int targetBoneIndex = -1;      // The bone index to aim at (-1 if none found)
        std::string targetBoneName;    // Name of the target bone
        std::vector<std::pair<int, std::string>> allBones; // All bones (index, name)
    };

    // Global storage for tracking dumped enemy types
    extern std::unordered_set<std::string> g_dumpedEnemyNames;
    
    // Global map for storing bone data per enemy type
    extern std::unordered_map<std::string, EnemyBoneData> g_enemyBoneMap;
    
    // File path for storing bone data
    extern std::string g_boneDataFilePath;

    // Initialize the bone analyzer (load existing data from file)
    void Initialize();

    // Save bone data to file
    void SaveBoneDataToFile();

    // Load bone data from file
    void LoadBoneDataFromFile();

    // Helper function to get a stable class name from an object by parsing GetFullName() and removing instance IDs
    std::string GetUObjectClassName(SDK::UObject* obj);

    // Helper function to build a unique enemy key from an enemy pawn and its skeletal mesh
    std::string BuildEnemyKey(SDK::AREnemyPawnBase* enemy, SDK::USkeletalMeshComponent* mesh);

    // Function to find the best target bone from a list of bones with prioritization
    // Priority: "head" -> "Body" -> "Core" -> "Spine_01" -> first bone (case insensitive matching)
    EnemyBoneData AnalyzeBones(SDK::USkeletalMeshComponent* mesh);

    // Function to dump bones for a single skeletal mesh component
    void DumpBones(SDK::USkeletalMeshComponent* mesh);

    // Main function to iterate enemies and dump bones only once per unique enemy "type"
    void DumpUniqueEnemyBones(SDK::UWorld* World);

    // Get the target bone index for a specific enemy type (for use by aimbot)
    int GetTargetBoneIndex(const std::string& enemyKey);

    // Get the target bone name for a specific enemy type (for debugging)
    std::string GetTargetBoneName(const std::string& enemyKey);

    // Automatically analyze and add a single enemy to the database
    bool AnalyzeAndAddEnemy(SDK::AREnemyPawnBase* enemy, SDK::USkeletalMeshComponent* mesh);

    // Display current bone database status (for debugging)
    void DisplayBoneDatabase();
}