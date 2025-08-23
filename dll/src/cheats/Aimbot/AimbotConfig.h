#pragma once

#include <Windows.h>

struct AimbotConfig {
    // Core Settings
    bool enabled = true;
    bool smoothEnabled = false;
    bool visibilityCheck = false;
    bool drawFOV = false;
    bool predictiveAiming = false; // Predict target movement
    
    // Input Keys
    BYTE triggerKey = VK_XBUTTON1; // Mouse4
    BYTE toggleKey = VK_INSERT;    // Toggle aimbot on/off
    
    // Targeting Parameters
    float maxDistance = 50000.0f;
    float fovRadius = 52800.0f;
    float smoothFactor = 8.0f;
    float maxTurnSpeed = 5180.0f;
    
    // Advanced Settings
    float reactionTime = 0.0f;    // Simulated human reaction time
    float targetSwitchDelay = 0.0f; // Delay when switching targets
    bool prioritizeHeadshots = false;
    bool aimAtMovingTargets = true;
    
    // Aim Zones (priority order)
    struct {
        bool head = true;
        bool chest = true;
        bool body = true;
    } aimZones;
    
    // Visual Settings
    struct {
        float r = 1.0f, g = 1.0f, b = 1.0f, a = 0.3f;
    } fovColor;
    
    struct {
        float r = 1.0f, g = 0.0f, b = 0.0f, a = 1.0f;
    } enemyColor;
    
    struct {
        float r = 0.0f, g = 1.0f, b = 0.0f, a = 1.0f;
    } targetColor;
    
    // Safety Features
    bool humanizeMovement = false;
    float maxAimSnapDistance = 1800.0f; // Max degrees to snap instantly
    bool requireLineOfSight = true;
};

extern AimbotConfig g_AimbotConfig;