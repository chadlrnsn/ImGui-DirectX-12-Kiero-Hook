#include <Windows.h>
#include <iostream>
#include <unordered_set>
#include <algorithm>
#include "SDK/Engine_classes.hpp"
#include "SDK/RGame_classes.hpp"
#include "Aimbot/AimbotController.h"
#include "Aimbot/AimbotConfig.h"
#include "SDK/BP_EngineRifle_Script_classes.hpp"
#include "SDK/Basic.hpp"
#include "SDK/RGame_parameters.hpp"
#include "Analysis/BoneAnalyzer.h"

// Global variables
bool f1KeyPressed = false;
bool g_shouldExit = false;
bool AutoCheatManager = true;
HMODULE g_hModule = nullptr;
DWORD lastPrintTime = 0;
using Kismet = SDK::UKismetSystemLibrary;

// Global weapon management variables to avoid redefinition
SDK::ARPlayerPawn* g_cachedCharacter = nullptr;
SDK::ARWeapon* g_cachedWeapon = nullptr;
SDK::URGWeaponScript* g_cachedWeaponScript = nullptr;
SDK::UBP_EngineRifle_Script_C* g_cachedEngineRifleScript = nullptr;
bool g_isEngineRifle = false;



void ConsoleWrite(const std::string text)
{
	std::cout << text << std::endl;
}

void ConsoleWrite(const std::wstring text)
{
	std::wcout << text << std::endl;
}

void DebugPrint(const std::string& text) {

	ConsoleWrite(text);

}
// Function to debug print all weapon settings
void PrintWeaponSettings(SDK::URBaseWeaponSettings* weaponSettings) {
	if (!weaponSettings) {
		std::cout << "ERROR: WeaponSettings is null!" << '\n';
		return;
	}

	std::cout << "\n=== CURRENT WEAPON SETTINGS DEBUG ===" << '\n';

	// Float weapon settings
	std::cout << "Float Weapon Settings:" << '\n';
	std::cout << "- BaseWeaponDamage: " << weaponSettings->BaseWeaponDamage.BaseValue
		<< " (Range: " << weaponSettings->BaseWeaponDamage.MinMaxRange.X
		<< " - " << weaponSettings->BaseWeaponDamage.MinMaxRange.Y << ")" << '\n';

	std::cout << "- BaseWeaponCriticalMultiplier: " << weaponSettings->BaseWeaponCriticalMultiplier.BaseValue
		<< " (Range: " << weaponSettings->BaseWeaponCriticalMultiplier.MinMaxRange.X
		<< " - " << weaponSettings->BaseWeaponCriticalMultiplier.MinMaxRange.Y << ")" << '\n';

	std::cout << "- BaseRateOfFire: " << weaponSettings->BaseRateOfFire.BaseValue
		<< " (Range: " << weaponSettings->BaseRateOfFire.MinMaxRange.X
		<< " - " << weaponSettings->BaseRateOfFire.MinMaxRange.Y << ")" << '\n';

	std::cout << "- BaseCooldown: " << weaponSettings->BaseCooldown.BaseValue
		<< " (Range: " << weaponSettings->BaseCooldown.MinMaxRange.X
		<< " - " << weaponSettings->BaseCooldown.MinMaxRange.Y << ")" << '\n';

	std::cout << "- BaseRecoil: " << weaponSettings->BaseRecoil.BaseValue
		<< " (Range: " << weaponSettings->BaseRecoil.MinMaxRange.X
		<< " - " << weaponSettings->BaseRecoil.MinMaxRange.Y << ")" << '\n';

	std::cout << "- BaseRecoilRecovery: " << weaponSettings->BaseRecoilRecovery.BaseValue
		<< " (Range: " << weaponSettings->BaseRecoilRecovery.MinMaxRange.X
		<< " - " << weaponSettings->BaseRecoilRecovery.MinMaxRange.Y << ")" << '\n';

	std::cout << "- BaseSpread: " << weaponSettings->BaseSpread.BaseValue
		<< " (Range: " << weaponSettings->BaseSpread.MinMaxRange.X
		<< " - " << weaponSettings->BaseSpread.MinMaxRange.Y << ")" << '\n';

	std::cout << "- BaseReloadTime: " << weaponSettings->BaseReloadTime.BaseValue
		<< " (Range: " << weaponSettings->BaseReloadTime.MinMaxRange.X
		<< " - " << weaponSettings->BaseReloadTime.MinMaxRange.Y << ")" << '\n';

	std::cout << "- BaseEffectiveRange: " << weaponSettings->BaseEffectiveRange.BaseValue
		<< " (Range: " << weaponSettings->BaseEffectiveRange.MinMaxRange.X
		<< " - " << weaponSettings->BaseEffectiveRange.MinMaxRange.Y << ")" << '\n';

	std::cout << "- BaseProcChance: " << weaponSettings->BaseProcChance.BaseValue
		<< " (Range: " << weaponSettings->BaseProcChance.MinMaxRange.X
		<< " - " << weaponSettings->BaseProcChance.MinMaxRange.Y << ")" << '\n';

	std::cout << "- BaseMinSpreadMultiplier: " << weaponSettings->BaseMinSpreadMultiplier.BaseValue
		<< " (Range: " << weaponSettings->BaseMinSpreadMultiplier.MinMaxRange.X
		<< " - " << weaponSettings->BaseMinSpreadMultiplier.MinMaxRange.Y << ")" << '\n';

	std::cout << "- BaseMaxSpreadMultiplier: " << weaponSettings->BaseMaxSpreadMultiplier.BaseValue
		<< " (Range: " << weaponSettings->BaseMaxSpreadMultiplier.MinMaxRange.X
		<< " - " << weaponSettings->BaseMaxSpreadMultiplier.MinMaxRange.Y << ")" << '\n';

	// Integer weapon settings
	std::cout << "\nInteger Weapon Settings:" << '\n';
	std::cout << "- BaseAmmoCost: " << weaponSettings->BaseAmmoCost.BaseValue
		<< " (Range: " << weaponSettings->BaseAmmoCost.MinMaxRange.X
		<< " - " << weaponSettings->BaseAmmoCost.MinMaxRange.Y << ")" << '\n';

	std::cout << "- BaseClipSize: " << weaponSettings->BaseClipSize.BaseValue
		<< " (Range: " << weaponSettings->BaseClipSize.MinMaxRange.X
		<< " - " << weaponSettings->BaseClipSize.MinMaxRange.Y << ")" << '\n';

	// Plain float settings
	std::cout << "\nPlain Float Settings:" << '\n';
	std::cout << "- ReloadTimeDelta: " << weaponSettings->ReloadTimeDelta << '\n';
	std::cout << "- ClipSizeDelta: " << weaponSettings->ClipSizeDelta << '\n';
	std::cout << "- ReduceProjectileSpreadTime: " << weaponSettings->ReduceProjectileSpreadTime << '\n';

	// Vector settings
	std::cout << "\nVector Settings:" << '\n';
	std::cout << "- ProjectileTraceOffset: (" << weaponSettings->ProjectileTraceOffset.X
		<< ", " << weaponSettings->ProjectileTraceOffset.Y
		<< ", " << weaponSettings->ProjectileTraceOffset.Z << ")" << '\n';

	std::cout << "- RecoilInterpSpeedRotationPosition: (" << weaponSettings->RecoilInterpSpeedRotationPosition.X
		<< ", " << weaponSettings->RecoilInterpSpeedRotationPosition.Y << ")" << '\n';

	std::cout << "- RecoilRotationRollMinMax: (" << weaponSettings->RecoilRotationRollMinMax.X
		<< ", " << weaponSettings->RecoilRotationRollMinMax.Y << ")" << '\n';

	std::cout << "- RecoilRotationPitchMinMax: (" << weaponSettings->RecoilRotationPitchMinMax.X
		<< ", " << weaponSettings->RecoilRotationPitchMinMax.Y << ")" << '\n';

	std::cout << "- RecoilRotationYawMinMax: (" << weaponSettings->RecoilRotationYawMinMax.X
		<< ", " << weaponSettings->RecoilRotationYawMinMax.Y << ")" << '\n';

	std::cout << "- RecoilPositionXMinMax: (" << weaponSettings->RecoilPositionXMinMax.X
		<< ", " << weaponSettings->RecoilPositionXMinMax.Y << ")" << '\n';

	std::cout << "- RecoilPositionYMinMax: (" << weaponSettings->RecoilPositionYMinMax.X
		<< ", " << weaponSettings->RecoilPositionYMinMax.Y << ")" << '\n';

	std::cout << "- RecoilPositionZMinMax: (" << weaponSettings->RecoilPositionZMinMax.X
		<< ", " << weaponSettings->RecoilPositionZMinMax.Y << ")" << '\n';

	// Animation overrides
	std::cout << "\nAnimation Overrides:" << '\n';
	std::cout << "- RecoilAnimOverride: " << weaponSettings->RecoilAnimOverride.ToString() << '\n';
	std::cout << "- ReloadAnimOverride: " << weaponSettings->ReloadAnimOverride.ToString() << '\n';

	std::cout << "=== END WEAPON SETTINGS DEBUG ===\n" << '\n';
}

// Function to update cached weapon references
void UpdateWeaponCache(SDK::ARPlayerPawn* character) {
	if (!character) {
		g_cachedCharacter = nullptr;
		g_cachedWeapon = nullptr;
		g_cachedWeaponScript = nullptr;
		g_cachedEngineRifleScript = nullptr;
		g_isEngineRifle = false;
		return;
	}

	// Update character cache
	g_cachedCharacter = character;

	// Update weapon cache
	auto weapon = character->GetEquippedWeapon();
	if (weapon != g_cachedWeapon) {
		g_cachedWeapon = weapon;
		g_cachedWeaponScript = nullptr;
		g_cachedEngineRifleScript = nullptr;
		g_isEngineRifle = false;

		if (weapon && weapon->RuntimeWeaponScript) {
			g_cachedWeaponScript = weapon->RuntimeWeaponScript;

			// Check if it's an Engine Rifle and cache the specialized script
			if (g_cachedWeaponScript->IsA(SDK::UBP_EngineRifle_Script_C::StaticClass())) {
				g_cachedEngineRifleScript = static_cast<SDK::UBP_EngineRifle_Script_C*>(g_cachedWeaponScript);
				g_isEngineRifle = true;
				std::cout << "[WEAPON] Engine Rifle detected and cached!" << '\n';
			}
		}
	}
}

// Function to manage Engine Rifle heat (called every frame)
void ManageEngineRifleHeat() {
	if (g_isEngineRifle && g_cachedEngineRifleScript) {
		// Force set heat to 0 to prevent overheating
		g_cachedEngineRifleScript->ForceSetHeat(0.0);
		//      std::cout << "Fullname of cached primary is: " << g_cachedEngineRifleScript->PrimaryWeaponModScript->GetFullName() 
				 // << std::endl;
		//      std::cout << "Fullname of cached secondary is: " << g_cachedEngineRifleScript->SecondaryWeaponModScript->IsSecondaryFire()
		//          << std::endl;
	}
}

// Function to enable and use console commands via CheatManager
void EnableConsoleAndExecuteCommands(SDK::APlayerController* PlayerController) {
	if (!PlayerController) {
		std::cout << "ERROR: PlayerController is null!" << '\n';
		return;
	}
	auto Engine = SDK::UEngine::GetEngine();
	if (!Engine) {
		std::cout << "ERROR: Engine not found!" << '\n';
		return;
	}
	// Enable console commands
	bool cheatSpawned = false;
	bool consoleSpawned = false;

	bool Cheat_spawned = false;
	bool Console_spawned = false;

	// Spawn CheatManager for the PlayerController
	if (!PlayerController->CheatManager && PlayerController->CheatClass)
	{
		if (auto CheatObject = SDK::UGameplayStatics::SpawnObject(PlayerController->CheatClass, PlayerController))
		{
			PlayerController->CheatManager = static_cast<SDK::UCheatManager*>(CheatObject);
			if (PlayerController->CheatManager)
			{
				PlayerController->EnableCheats();
				std::cout << "CheatManager spawned: " << PlayerController->CheatManager->GetFullName() << "\n";
				Cheat_spawned = true;
			}
		}
	}
	else
	{
		if (PlayerController->CheatManager)
		{
			Cheat_spawned = true;
		}
	}

	// Spawn Console for the GameViewport
	if (Engine->ConsoleClass && Engine->GameViewport && !Engine->GameViewport->ViewportConsole)
	{
		if (auto ConsoleObject = SDK::UGameplayStatics::SpawnObject(Engine->ConsoleClass, Engine->GameViewport))
		{
			if (auto ConsoleInstance = static_cast<SDK::UConsole*>(ConsoleObject))
			{
				Engine->GameViewport->ViewportConsole = ConsoleInstance;
				std::cout << "Console spawned: " << ConsoleInstance->GetFullName() << "\n";
				Console_spawned = true;
			}
		}
	}
	else
	{
		if (Engine->GameViewport->ViewportConsole)
		{
			Console_spawned = true;
		}
	}

	if (Cheat_spawned && Console_spawned)
	{
		AutoCheatManager = false;
		DebugPrint("Enabling cheats commands!");
		PlayerController->CheatManager->God();
		PlayerController->CheatManager->Slomo(2);
	}
	else
	{
		if (!Cheat_spawned)
		{
			DebugPrint("Failed to spawn CheatManager!");
		}
		if (!Console_spawned)
		{
			DebugPrint("Failed to spawn ViewPort Console!");
		}
		AutoCheatManager = true;
	}


}

void HandleDebugOutputOld(SDK::UWorld* World) {
	std::cout << "\n=== F1 PRESSED - CHEAT COMMANDS DEMO ===" << '\n';

	auto playerController = World->OwningGameInstance->LocalPlayers[0]->PlayerController;
	if (!playerController) {
		std::cout << "ERROR: PlayerController not found!" << '\n';
		return;
	}
	// playerController->CheatManager->DestroyPawns(SDK::AREnemyPawnBase::StaticClass());

	auto character = static_cast<SDK::ARPlayerPawn*>(playerController->K2_GetPawn());
	if (!character) {
		std::cout << "ERROR: Player Pawn not spawned yet!" << '\n';
		return;
	}

	// Weapon modifications (keeping existing functionality)
	auto weapon = character->GetEquippedWeapon();
	if (weapon && weapon->RuntimeWeaponScript) {
		std::cout << "\nApplying weapon modifications..." << '\n';

		auto weaponScript = weapon->RuntimeWeaponScript;
		weaponScript->MulticastSetAmmoInClip(1000);

		auto weaponSettings = weaponScript->GetBaseWeaponSettings();
		if (weaponSettings) {
			// Cheat weapon values
			weaponSettings->BaseAmmoCost.BaseValue = 0;
			weaponSettings->BaseWeaponDamage.BaseValue = 500.0f;
			weaponSettings->BaseRateOfFire.BaseValue = 30.0f;
			weaponSettings->BaseRecoil.BaseValue = 0.0f;
			weaponSettings->BaseSpread.BaseValue = 0.0f;
			weaponSettings->BaseReloadTime.BaseValue = 0.01f;

			weaponScript->SetBaseWeaponSettings(weaponSettings);
			std::cout << "? Weapon modifications applied!" << '\n';
		}
	}
}

void HandleWeaponMods(SDK::UWorld* World) {
	std::cout << "\n=== F1 PRESSED - CURRENT ACTORS LIST ===" << '\n';

	auto playerController = World->OwningGameInstance->LocalPlayers[0]->PlayerController;
	if (!playerController) {
		std::cout << "ERROR: PlayerController not found!" << '\n';
		return;
	}

	auto character = static_cast<SDK::ARPlayerPawn*>(playerController->K2_GetPawn());
	if (!character) {
		std::cout << "ERROR: Player Pawn not spawned yet!" << '\n';
		return;
	}
	auto weapon = character->GetEquippedWeapon();
	if (!weapon) {
		std::cout << "ERROR: Player weapon not found!" << '\n';
		return;
	}

	std::cout << "Current ammo: " << weapon->RuntimeWeaponScript->GetAmmoInClip() << '\n';
	auto weaponScript = weapon->RuntimeWeaponScript;
	if (!weaponScript) {
		std::cout << "ERROR: Weapon Runtime Script not found!" << '\n';
		return;
	}

	auto weaponSettings = weaponScript->GetBaseWeaponSettings();

	// Print all weapon settings before modification
	PrintWeaponSettings(weaponSettings);

	std::cout << "Current Weapon Settings:" << '\n';
	std::cout << "- Ammo cost: " << weaponSettings->BaseAmmoCost.BaseValue << '\n';
	SDK::URBaseWeaponSettings* newBaseSettings = weaponScript->GetBaseWeaponSettings();

	if (weaponScript->IsA(SDK::UBP_EngineRifle_Script_C::StaticClass())) {
		std::cout << "Weapon is a BP_EngineRifle_Script" << '\n';
		auto engineRifleScript = static_cast<SDK::UBP_EngineRifle_Script_C*>(weaponScript);
		std::cout << "Current heat: " << engineRifleScript->CurrentHeat << '\n';
	}
	else {
		std::cout << "Weapon is NOT a BP_EngineRifle_Script" << '\n';
	}
	// Set cheaty weapon values
	std::cout << "Applying cheater weapon modifications..." << '\n';

	// Infinite ammo (no ammo cost)
	newBaseSettings->BaseAmmoCost.BaseValue = 0;
	std::cout << "- Set ammo cost to 0 (infinite ammo)" << '\n';

	// Massively increased damage
	newBaseSettings->BaseWeaponDamage.BaseValue = 200.0f;
	newBaseSettings->BaseWeaponDamage.MinMaxRange.X = 0.0f;
	newBaseSettings->BaseWeaponDamage.MinMaxRange.Y = 1000.0f;
	std::cout << "- Set weapon damage to 200" << '\n';

	// Extremely high critical hit multiplier
	newBaseSettings->BaseWeaponCriticalMultiplier.BaseValue = 5.0f;
	newBaseSettings->BaseWeaponCriticalMultiplier.MinMaxRange.X = 0.0f;
	newBaseSettings->BaseWeaponCriticalMultiplier.MinMaxRange.Y = 9999.0f;
	std::cout << "- Set critical multiplier to 5x" << '\n';

	// Super fast rate of fire
	newBaseSettings->BaseRateOfFire.BaseValue = 20.0f;
	newBaseSettings->BaseRateOfFire.MinMaxRange.X = 1.0f;
	newBaseSettings->BaseRateOfFire.MinMaxRange.Y = 20.0f;
	std::cout << "- Set rate of fire to 20" << '\n';

	// No cooldown
	newBaseSettings->BaseCooldown.BaseValue = 0.0f;
	newBaseSettings->BaseCooldown.MinMaxRange.X = 0.0f;
	newBaseSettings->BaseCooldown.MinMaxRange.Y = 0.0f;
	std::cout << "- Set cooldown to 0" << '\n';

	// No recoil
	newBaseSettings->BaseRecoil.BaseValue = 0.0f;
	newBaseSettings->BaseRecoil.MinMaxRange.X = 0.0f;
	newBaseSettings->BaseRecoil.MinMaxRange.Y = 0.0f;
	std::cout << "- Set recoil to 0" << '\n';

	// Instant recoil recovery
	newBaseSettings->BaseRecoilRecovery.BaseValue = 100.0f;
	newBaseSettings->BaseRecoilRecovery.MinMaxRange.X = 0.0f;
	newBaseSettings->BaseRecoilRecovery.MinMaxRange.Y = 9999.0f;
	std::cout << "- Set recoil recovery to 100" << '\n';

	// No spread (perfect accuracy)
	newBaseSettings->BaseSpread.BaseValue = 0.0f;
	newBaseSettings->BaseSpread.MinMaxRange.X = 0.0f;
	newBaseSettings->BaseSpread.MinMaxRange.Y = 0.0f;
	std::cout << "- Set spread to 0 (perfect accuracy)" << '\n';

	// Instant reload
	newBaseSettings->BaseReloadTime.BaseValue = 0.01f;
	newBaseSettings->BaseReloadTime.MinMaxRange.X = 0.01f;
	newBaseSettings->BaseReloadTime.MinMaxRange.Y = 0.01f;
	newBaseSettings->ReloadTimeDelta = 0.0f;
	std::cout << "- Set reload time to 0.01 seconds" << '\n';

	// Apply the modified settings
	weaponScript->SetBaseWeaponSettings(newBaseSettings);
	std::cout << "Cheater weapon modifications applied successfully!" << '\n';
	std::cout << "Cached engine recoil value: " << g_cachedEngineRifleScript->GetBaseWeaponSettings()->BaseRecoil.BaseValue <<
		'\n';
	auto secondBaseSettings = g_cachedEngineRifleScript->SecondaryWeaponModScript->GetBaseWeaponSettings();
	std::cout << "Secondary Weapon Settings:" << '\n';
	PrintWeaponSettings(secondBaseSettings);

	// Apply the modified settings to the secondary weapon mod script
	g_cachedEngineRifleScript->SecondaryWeaponModScript->ApplyFireSettings(newBaseSettings);
	PrintWeaponSettings(secondBaseSettings);
}


// Function to handle clean uninjection
void HandleUninjection() {
	std::cout << "\n=== UNINJECTION REQUESTED ===" << '\n';
	std::cout << "Initiating clean shutdown sequence..." << '\n';

	// Set the exit flag to break the main loop
	g_shouldExit = true;
}

// Function to cleanup and free console safely
void CleanupConsole() {
	std::cout << "Cleaning up console..." << '\n';
	std::cout.flush();
	std::cerr.flush();

	// Give a small delay to ensure the message is displayed
	Sleep(100);

	// Simply free the console without trying to redirect streams
	// The streams will be automatically cleaned up when the console is freed
	FreeConsole();
}

// Function to perform complete cleanup
void PerformCleanup() {
	// Shutdown aimbot system
	AimbotController::Shutdown();

}

// Function to initialize aimbot configuration
void InitializeAimbotConfig() {
	// Set default aimbot configuration
	g_AimbotConfig.enabled = true;
	g_AimbotConfig.smoothEnabled = false;
	g_AimbotConfig.visibilityCheck = false;
	g_AimbotConfig.drawFOV = false;
	g_AimbotConfig.predictiveAiming = false;
	g_AimbotConfig.humanizeMovement = false;

	g_AimbotConfig.triggerKey = VK_XBUTTON1; // Mouse4
	g_AimbotConfig.toggleKey = VK_INSERT;    // Insert key
	g_AimbotConfig.maxDistance = 50000.0f;
	g_AimbotConfig.fovRadius = 45000.0f;
	g_AimbotConfig.smoothFactor = 8.0f;
	g_AimbotConfig.maxTurnSpeed = 180.0f;
	g_AimbotConfig.reactionTime = 0.00f;
	g_AimbotConfig.targetSwitchDelay = 0.1f;
	g_AimbotConfig.prioritizeHeadshots = false;
	g_AimbotConfig.maxAimSnapDistance = 9000.0f;

	std::cout << "Professional Aimbot Configuration:" << '\n';
	std::cout << "- Status: " << (g_AimbotConfig.enabled ? "ENABLED" : "DISABLED") << '\n';
	std::cout << "- Trigger Key: Mouse4 (Hold to aim)" << '\n';
	std::cout << "- Toggle Key: Insert (Toggle on/off)" << '\n';
	std::cout << "- Max Target Distance: " << g_AimbotConfig.maxDistance << " units" << '\n';
	std::cout << "- FOV Radius: " << g_AimbotConfig.fovRadius << " degrees" << '\n';
	std::cout << "- Smooth Aiming: " << (g_AimbotConfig.smoothEnabled ? "Enabled" : "Disabled") << '\n';
	std::cout << "- Humanized Movement: " << (g_AimbotConfig.humanizeMovement ? "Enabled" : "Disabled") << '\n';
	std::cout << "- Visibility Check: " << (g_AimbotConfig.visibilityCheck ? "Enabled" : "Disabled") << '\n';
	std::cout << "- Max Turn Speed: " << g_AimbotConfig.maxTurnSpeed << " deg/s" << '\n';
	std::cout << "- Reaction Time: " << g_AimbotConfig.reactionTime << "s" << '\n';
}

using tProcessEvent = void(*)(const SDK::UObject*, SDK::UFunction*, void*);
static tProcessEvent g_oProcessEvent = nullptr;
static SDK::UFunction* g_FireProjectileFn = nullptr;


DWORD MainThread(HMODULE Module)
{
	// Store module handle for cleanup
	g_hModule = Module;

	/* Code to open a console window */
	AllocConsole();
	FILE* Dummy;
	freopen_s(&Dummy, "CONOUT$", "w", stdout);
	freopen_s(&Dummy, "CONIN$", "r", stdin);

	std::cout << "====================================================" << '\n';
	std::cout << "    Abyssus Hack    " << '\n';
	std::cout << "====================================================" << '\n';
	std::cout << '\n';
	std::cout << "Controls:" << '\n';
	std::cout << "- F1: Debug enemy actor list" << '\n';
	std::cout << "- F2: Clean uninject and exit" << '\n';
	std::cout << "- F3: Dump enemy bones (unique per enemy name)" << '\n';
	std::cout << "- F4: Display bone database status" << '\n';
	std::cout << "- Mouse4: Hold to activate aimbot" << '\n';
	std::cout << "- Insert: Toggle aimbot on/off" << '\n';
	std::cout << '\n';

	// Initialize game world access
	SDK::UWorld* World = SDK::UWorld::GetWorld();
	if (!World) {
		std::cout << "ERROR: Could not get world instance!" << '\n';
		return 0;
	}

	auto level = World->PersistentLevel;
	if (!level) {
		std::cout << "ERROR: Could not get persistent level!" << '\n';
		return 0;
	}

	// Validate game instance and player controller
	if (!World->OwningGameInstance || World->OwningGameInstance->LocalPlayers.Num() == 0) {
		std::cout << "ERROR: Could not get game instance or local players!" << '\n';
		return 0;
	}

	auto playerController = World->OwningGameInstance->LocalPlayers[0]->PlayerController;
	if (!playerController) {
		std::cout << "ERROR: PlayerController not found!" << '\n';
		return 0;
	}
	if (AutoCheatManager)
		EnableConsoleAndExecuteCommands(playerController);

	std::cout << "Game Connection Status:" << '\n';
	std::cout << "- World Instance: Connected" << '\n';
	std::cout << "- Player Controller: " << playerController->GetName() << '\n';

	auto character = static_cast<SDK::ARPlayerPawn*>(playerController->K2_GetPawn());
	if (!character) {
		std::cout << "- Player Pawn: Not spawned yet" << '\n';
	}
	else {
		std::cout << "- Player Pawn: " << character->GetName() << '\n';
	}

	// Initialize aimbot system
	std::cout << '\n';
	InitializeAimbotConfig();
	AimbotController::Initialize();

	// Initialize bone analyzer system
	BoneAnalyzer::Initialize();

	std::cout << '\n' << "=== SYSTEM READY FOR OPERATION ===" << '\n';
	std::cout << "Press F2 to safely uninject and exit" << '\n';
	lastPrintTime = GetTickCount();

	bool f1WasPressed = false;
	bool f2WasPressed = false;
	bool f3WasPressed = false;
	bool f4WasPressed = false;
	DWORD lastFrameTime = GetTickCount();

	// Main loop - now with exit condition
	while (!g_shouldExit)
	{
		DWORD currentTime = GetTickCount();
		float deltaTime = (currentTime - lastFrameTime) / 1000.0f; // Convert to seconds
		lastFrameTime = currentTime;

		// Refresh character reference if needed
		character = static_cast<SDK::ARPlayerPawn*>(playerController->K2_GetPawn());

		// Update weapon cache and manage Engine Rifle heat
		UpdateWeaponCache(character);
		ManageEngineRifleHeat();

		// God mode - keep player health at max
		if (character && character->HealthComponent) {
			character->HealthComponent->currentHealth = character->HealthComponent->GetMaxHealth();
		}

		// Handle F1 debug key
		bool f1CurrentlyPressed = (GetAsyncKeyState(VK_F1) & 0x8000) != 0;
		if (f1CurrentlyPressed && !f1WasPressed) {
			HandleWeaponMods(World);
		}
		f1WasPressed = f1CurrentlyPressed;

		// Handle F2 uninject key
		bool f2CurrentlyPressed = (GetAsyncKeyState(VK_F2) & 0x8000) != 0;
		if (f2CurrentlyPressed && !f2WasPressed) {
			HandleUninjection();
		}
		f2WasPressed = f2CurrentlyPressed;

		// Handle F3 dump enemy bones key
		bool f3CurrentlyPressed = (GetAsyncKeyState(VK_F3) & 0x8000) != 0;
		if (f3CurrentlyPressed && !f3WasPressed) {
			BoneAnalyzer::DumpUniqueEnemyBones(World);
		}
		f3WasPressed = f3CurrentlyPressed;

		// Handle F4 display bone database key
		bool f4CurrentlyPressed = (GetAsyncKeyState(VK_F4) & 0x8000) != 0;
		if (f4CurrentlyPressed && !f4WasPressed) {
			BoneAnalyzer::DisplayBoneDatabase();
		}
		f4WasPressed = f4CurrentlyPressed;

		// Update aimbot system
		AimbotController::Update(deltaTime);

		// Small delay to prevent excessive CPU usage while maintaining responsiveness
		Sleep(5); // Increased responsiveness for better aimbot performance
	}

	std::cout << "\nExiting main loop..." << '\n';

	// Perform cleanup
	PerformCleanup();
	fclose(Dummy);
	FreeConsole();
	// Create a new thread to handle DLL unloading
	// This prevents potential deadlocks when calling FreeLibrary from DllMain
	CreateThread(nullptr, 0, [](LPVOID param) -> DWORD {
		Sleep(100); // Small delay to ensure cleanup is complete
		FreeLibraryAndExitThread(static_cast<HMODULE>(param), 0);
		return 0;
		}, g_hModule, 0, nullptr);

	return 0;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD reason, LPVOID lpReserved)
{
	switch (reason)
	{
	case DLL_PROCESS_ATTACH:
		// Disable DLL_THREAD_ATTACH and DLL_THREAD_DETACH notifications for performance
		DisableThreadLibraryCalls(hModule);
		CreateThread(0, 0, (LPTHREAD_START_ROUTINE)MainThread, hModule, 0, 0);
		break;
	case DLL_PROCESS_DETACH:
		// Set exit flag in case of forced detach
		g_shouldExit = true;
		break;
	}

	return TRUE;
}