#pragma once

#include "../SDK/Engine_classes.hpp"
#include "../SDK/RGame_classes.hpp"

namespace Math {
    // Basic vector math utilities - now using UE built-in functions for performance
    
    float Distance(const SDK::FVector& a, const SDK::FVector& b);
    float Distance2D(const SDK::FVector& a, const SDK::FVector& b);
    
    SDK::FVector Subtract(const SDK::FVector& a, const SDK::FVector& b);
    SDK::FVector Add(const SDK::FVector& a, const SDK::FVector& b);
    SDK::FVector Multiply(const SDK::FVector& a, float scalar);
    SDK::FVector Normalize(const SDK::FVector& vector);
    
    float DotProduct(const SDK::FVector& a, const SDK::FVector& b);
    float VectorLength(const SDK::FVector& vector);
    
    // Screen projection using UE built-in functions
    bool WorldToScreen(const SDK::FVector& worldPos, SDK::FVector2D* screenPos, 
                      SDK::APlayerController* playerController);
    
    // Helper function to get screen center with proper viewport handling
    SDK::FVector2D GetScreenCenter(SDK::APlayerController* playerController);
    
    // Rotation utilities using UKismetMathLibrary for accuracy
    SDK::FRotator VectorToRotation(const SDK::FVector& vector);
    SDK::FVector RotationToVector(const SDK::FRotator& rotation);
    
    // Calculate angular distance between two rotations
    float GetAngleBetweenRotations(const SDK::FRotator& rot1, const SDK::FRotator& rot2);
    
    // Smooth rotation interpolation using UKismetMathLibrary
    SDK::FRotator LerpRotation(const SDK::FRotator& from, const SDK::FRotator& to, 
                              float deltaTime, float speed);
    
    // FOV checking - both 3D world space and screen space versions
    bool IsWithinFOV(const SDK::FVector& targetPos, const SDK::FVector& cameraPos, 
                     const SDK::FRotator& cameraRotation, float fovRadius);
    
    // Enhanced FOV check using screen projection for more accurate targeting
    bool IsWithinScreenFOV(const SDK::FVector& targetPos, const SDK::FVector& cameraPos,
                          SDK::APlayerController* playerController, float fovRadius);
    
    // UKismetMathLibrary wrapper functions for common operations
    SDK::FRotator CalculateLookAtRotation(const SDK::FVector& start, const SDK::FVector& target);
    SDK::FRotator CalculateRotationDelta(const SDK::FRotator& current, const SDK::FRotator& target);
    
    // Utility functions
    float NormalizeAngle(float angle);
    float ClampAngle(float angle, float min, float inmax);
}