#include "CheatMain.h"
#include "Config.h"
#include "../Features/WeaponService.h"
#include "../Aimbot/AimbotController.h"
#include "../Analysis/BoneAnalyzer.h"
#include "../Utils/Console.h"
#include "../Utils/Input.h"
#include "../Services/GameServices.h"
#include "../Services/PlayerEffectsService.h"
#include <dev/logger.h>
#include <iostream>
#include <mutex>

// Mutex for thread-safe target list access (global scope)
std::mutex list_mutex;

namespace Cheat {
    namespace Core {
        
        bool CheatMain::Initialize() {
            if (Config::System::Initialized) {
                LOG_INFO("CheatMain already initialized");
                return true;
            }

            LOG_INFO("Initializing cheat system...");

            // Initialize centralized configuration
            Config::Initialize();

            // Initialize game connection
            if (!UpdateSDK(true)) {
                LOG_ERROR("Failed to initialize game connection");
                return false;
            }

            // Initialize subsystems
            if (!InitializeSubsystems()) {
                LOG_ERROR("Failed to initialize subsystems");
                return false;
            }

            Config::System::LastFrameTime = GetTickCount();
            Config::System::Initialized = true;

            LOG_INFO("=== CHEAT SYSTEM READY FOR OPERATION ===");
            Config::PrintConfiguration();

            return true;
        }
        
        void CheatMain::Update(DWORD tick) {
            if (!Config::System::Initialized) {
                return;
            }

            // Update SDK without logging each frame
            if (!UpdateSDK(false)) {
                return;
            }

            // Fetch entities
            FetchEntities();

            // Process input
            ProcessInput();

            // Update subsystems with calculated deltaTime
            DWORD currentTime = GetTickCount();
            float deltaTime = (currentTime - Config::System::LastFrameTime) / 1000.0f;
            UpdateSubsystems(deltaTime);
            Config::System::LastFrameTime = currentTime;
        }
        
        void CheatMain::Shutdown() {
            if (!Config::System::Initialized) {
                return;
            }

            LOG_INFO("Shutting down cheat system...");

            // Shutdown subsystems
            AimbotController::Shutdown();

            Config::System::Initialized = false;
            Config::System::ShouldExit = false;
            Config::ClearGameState();

            LOG_INFO("Cheat system shutdown complete");
        }
        
        bool CheatMain::UpdateSDK(bool log) {
            // Delegate to GameServices facade; it also updates legacy Config::GameState during transition
            return Cheat::Services::GameServices::Refresh(log);
        }
        
        void CheatMain::FetchFromActors(std::vector<SDK::AActor*>* list) {
            if (Config::GameState::g_pWorld->Levels.Num() == 0)
                return;

            SDK::ULevel* currLevel = Config::GameState::g_pWorld->Levels[0];
            if (!currLevel)
                return;

            list->clear();

            for (int j = 0; j < currLevel->Actors.Num(); j++) {
                SDK::AActor* currActor = currLevel->Actors[j];

                if (!currActor)
                    continue;
                if(currActor->IsA(SDK::ARPlayerPawn::StaticClass())) 
                    continue;
                

                // Check for enemy pawns (adjust class name as needed for your game)
                if (currActor->IsA(SDK::AREnemyPawnBase::StaticClass())) {
                    list->push_back(currActor);
                }
            }
        }

        void CheatMain::FetchEntities() {
            if (!Config::GameState::g_pWorld ||
                !Config::GameState::g_pEngine ||
                !Config::GameState::g_pMyController ||
                !Config::GameState::g_pMyPawn) {
                return;
            }

            if (!Config::GameState::g_pWorld->GameState) {
                return;
            }

            std::vector<SDK::AActor*> newTargets;
            FetchFromActors(&newTargets);

            {
                std::lock_guard<std::mutex> lock(list_mutex);
                Config::GameState::g_TargetsList = std::move(newTargets);
            }
        }

        bool CheatMain::InitializeSubsystems() {
            // Always initialize console and cheat manager - it's required for many cheats
            if (Config::GameState::g_pMyController) {
                if (!Utils::Console::EnableCheatManager(Config::GameState::g_pMyController)) {
                    LOG_INFO("Failed to enable cheat manager - will retry during updates");
                } else {
                    LOG_INFO("CheatManager enabled successfully at startup");
                }
            }

            // Initialize aimbot system
            AimbotController::Initialize();

            // Initialize bone analyzer system
            BoneAnalyzer::Initialize();

            // Initialize weapon service
            Features::WeaponService::Initialize();

            return true;
        }
        
        void CheatMain::ProcessInput() {

            // Handle dump enemy bones key
            if (Utils::Input::IsKeyPressed(Config::Hotkeys::DumpBones)) {
                BoneAnalyzer::DumpUniqueEnemyBones(Config::GameState::g_pWorld);
            }

            // Handle display bone database key
            if (Utils::Input::IsKeyPressed(Config::Hotkeys::ShowBoneDB)) {
                BoneAnalyzer::DisplayBoneDatabase();
            }

            // Handle log weapon stats key
            if (Utils::Input::IsKeyPressed(Config::Hotkeys::LogWeaponStats)) {
                Features::WeaponService::LogAllWeaponStats();
            }
        }

        void CheatMain::UpdateSubsystems(float deltaTime) {
            // Update cheat toggles (God Mode, Speed Hack, etc.)
            Cheat::Services::PlayerEffectsService::Update(Cheat::Services::GameServices::GetPlayerController());

            // Update weapon service
            Features::WeaponService::Update();

            // Update aimbot system
            AimbotController::Update(deltaTime);
        }
        
    } // namespace Core
} // namespace Cheat
