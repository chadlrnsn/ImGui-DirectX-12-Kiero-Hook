#include "MathUtils.h"
#include <cmath>
#include <algorithm>
#include <iomanip>

#include "AimbotConfig.h"
#define NOMINMAX
namespace Math {
    
    float Distance(const SDK::FVector& a, const SDK::FVector& b) {
        // Use built-in UE function for better performance
        float distance = a.GetDistanceTo(b);
        return distance;
    }
    
    float Distance2D(const SDK::FVector& a, const SDK::FVector& b) {
        float dx = a.X - b.X;
        float dy = a.Y - b.Y;
        float distance = std::sqrt(dx * dx + dy * dy);
        return distance;
    }
    
    SDK::FVector Subtract(const SDK::FVector& a, const SDK::FVector& b) {
        // Use built-in UE operator
        return a - b;
    }
    
    SDK::FVector Add(const SDK::FVector& a, const SDK::FVector& b) {
        // Use built-in UE operator
        return a + b;
    }
    
    SDK::FVector Multiply(const SDK::FVector& a, float scalar) {
        // Use built-in UE operator
        return a * scalar;
    }
    
    SDK::FVector Normalize(const SDK::FVector& vector) {
        // Use built-in UE function for better performance and accuracy
        return vector.GetNormalized();
    }
    
    float DotProduct(const SDK::FVector& a, const SDK::FVector& b) {
        // Use built-in UE function
        return a.Dot(b);
    }
    
    float VectorLength(const SDK::FVector& vector) {
        // Calculate using distance to origin for consistency
        return vector.GetDistanceTo(SDK::FVector(0.0f, 0.0f, 0.0f));
    }
    
    bool WorldToScreen(const SDK::FVector& worldPos, SDK::FVector2D* screenPos, 
                      SDK::APlayerController* playerController) {
        if (!playerController || !screenPos) {
            std::cout << "[MATH] WorldToScreen failed: " << (!playerController ? "playerController is null" : "screenPos is null") << std::endl;
            return false;
        }
        
        // Use built-in UE function for world to screen projection
        bool result = playerController->ProjectWorldLocationToScreen(worldPos, screenPos, true);
        
        if (result) {
            std::cout << "[MATH] WorldToScreen success: (" << std::fixed << std::setprecision(1) 
                      << worldPos.X << ", " << worldPos.Y << ", " << worldPos.Z << ") -> (" 
                      << screenPos->X << ", " << screenPos->Y << ")" << std::endl;
        } else {
            std::cout << "[MATH] WorldToScreen failed: world position not on screen" << std::endl;
        }
        
        return result;
    }
    
    SDK::FRotator VectorToRotation(const SDK::FVector& vector) {
        // Use UKismetMathLibrary for accurate conversion
        SDK::FRotator rotation = SDK::UKismetMathLibrary::Conv_VectorToRotator(vector);
        std::cout << "[MATH] VectorToRotation: (" << std::fixed << std::setprecision(1) 
                  << vector.X << ", " << vector.Y << ", " << vector.Z << ") -> (" 
                  << rotation.Pitch << ", " << rotation.Yaw << ", " << rotation.Roll << ")" << std::endl;
        return rotation;
    }
    
    SDK::FVector RotationToVector(const SDK::FRotator& rotation) {
        // Use UKismetMathLibrary for accurate conversion
        return SDK::UKismetMathLibrary::Conv_RotatorToVector(rotation);
    }
    
    float GetAngleBetweenRotations(const SDK::FRotator& rot1, const SDK::FRotator& rot2) {
        // Use UKismetMathLibrary for normalized delta calculation
        SDK::FRotator deltaRotation = SDK::UKismetMathLibrary::NormalizedDeltaRotator(rot2, rot1);
        float angle = std::sqrt(deltaRotation.Yaw * deltaRotation.Yaw + deltaRotation.Pitch * deltaRotation.Pitch);
        
        std::cout << "[MATH] GetAngleBetweenRotations:" << std::endl;
        std::cout << "[MATH] - From: (" << std::fixed << std::setprecision(1) 
                  << rot1.Pitch << ", " << rot1.Yaw << ", " << rot1.Roll << ")" << std::endl;
        std::cout << "[MATH] - To: (" << std::fixed << std::setprecision(1) 
                  << rot2.Pitch << ", " << rot2.Yaw << ", " << rot2.Roll << ")" << std::endl;
        std::cout << "[MATH] - Delta: (" << std::fixed << std::setprecision(1) 
                  << deltaRotation.Pitch << ", " << deltaRotation.Yaw << ", " << deltaRotation.Roll << ")" << std::endl;
        std::cout << "[MATH] - Angle: " << std::fixed << std::setprecision(1) << angle << " degrees" << std::endl;
        
        return angle;
    }
    
    SDK::FRotator LerpRotation(const SDK::FRotator& from, const SDK::FRotator& to, 
                              float deltaTime, float speed) {
        // Use UKismetMathLibrary for proper rotation interpolation
        SDK::FRotator result = SDK::UKismetMathLibrary::RInterpTo(from, to, deltaTime, speed);
        
        std::cout << "[MATH] LerpRotation:" << std::endl;
        std::cout << "[MATH] - From: (" << std::fixed << std::setprecision(1) 
                  << from.Pitch << ", " << from.Yaw << ", " << from.Roll << ")" << std::endl;
        std::cout << "[MATH] - To: (" << std::fixed << std::setprecision(1) 
                  << to.Pitch << ", " << to.Yaw << ", " << to.Roll << ")" << std::endl;
        std::cout << "[MATH] - DeltaTime: " << std::fixed << std::setprecision(3) << deltaTime << "s" << std::endl;
        std::cout << "[MATH] - Speed: " << std::fixed << std::setprecision(1) << speed << " deg/s" << std::endl;
        std::cout << "[MATH] - Result: (" << std::fixed << std::setprecision(1) 
                  << result.Pitch << ", " << result.Yaw << ", " << result.Roll << ")" << std::endl;
        
        return result;
    }
    
    bool IsWithinFOV(const SDK::FVector& targetPos, const SDK::FVector& cameraPos, 
                     const SDK::FRotator& cameraRotation, float fovRadius) {
        // Calculate the rotation needed to look at the target
        SDK::FRotator targetRotation = SDK::UKismetMathLibrary::FindLookAtRotation(cameraPos, targetPos);
        
        // Calculate the delta rotation needed
        SDK::FRotator deltaRotation = SDK::UKismetMathLibrary::NormalizedDeltaRotator(targetRotation, cameraRotation);
        
        // Calculate the angular distance
        float angle = std::sqrt(deltaRotation.Yaw * deltaRotation.Yaw + deltaRotation.Pitch * deltaRotation.Pitch);
        bool withinFOV = angle <= fovRadius;
        
        std::cout << "[MATH] IsWithinFOV:" << std::endl;
        std::cout << "[MATH] - Camera pos: (" << std::fixed << std::setprecision(1) 
                  << cameraPos.X << ", " << cameraPos.Y << ", " << cameraPos.Z << ")" << std::endl;
        std::cout << "[MATH] - Target pos: (" << std::fixed << std::setprecision(1) 
                  << targetPos.X << ", " << targetPos.Y << ", " << targetPos.Z << ")" << std::endl;
        std::cout << "[MATH] - Camera rotation: (" << std::fixed << std::setprecision(1) 
                  << cameraRotation.Pitch << ", " << cameraRotation.Yaw << ", " << cameraRotation.Roll << ")" << std::endl;
        std::cout << "[MATH] - Target rotation: (" << std::fixed << std::setprecision(1) 
                  << targetRotation.Pitch << ", " << targetRotation.Yaw << ", " << targetRotation.Roll << ")" << std::endl;
        std::cout << "[MATH] - Angle: " << std::fixed << std::setprecision(1) << angle 
                  << " degrees (FOV limit: " << fovRadius << ")" << std::endl;
        std::cout << "[MATH] - Result: " << (withinFOV ? "WITHIN FOV" : "OUTSIDE FOV") << std::endl;
        
        return withinFOV;
    }
    
    float NormalizeAngle(float angle) {
        // Use UKismetMathLibrary for proper angle normalization
        return SDK::UKismetMathLibrary::NormalizeAxis(angle);
    }
    
    float ClampAngle(float angle, float inmin, float inmax) {

        return max(inmin, min(inmax, angle));
    }
    
    // Helper function to get Windows DPI scaling factor
    float GetWindowsScalingFactor()
    {
        HDC hdc = GetDC(NULL);
        if (!hdc) return 1.0f;
        
        float dpiX = static_cast<float>(GetDeviceCaps(hdc, LOGPIXELSX));
        ReleaseDC(NULL, hdc);
        
        // Standard DPI is 96, so scaling factor = current DPI / 96
        float scalingFactor = dpiX / 96.0f;
        std::cout << "[MATH] Windows DPI scaling factor: " << std::fixed << std::setprecision(2) << scalingFactor << std::endl;
        return scalingFactor;
    }

    // Helper function to get dynamic screen center with Windows scaling compensation
    SDK::FVector2D GetScreenCenter(SDK::APlayerController* playerController)
    {
        float scalingFactor = GetWindowsScalingFactor();
        if (!playerController)
        { 
            SDK::FVector2D defaultCenter(2560.0f / 2.0f, 1440.0f / 2.0f); // 4k resolution, 150% res scaling
            std::cout << "[MATH] GetScreenCenter (no controller): using default (" 
                      << std::fixed << std::setprecision(1) << defaultCenter.X << ", " << defaultCenter.Y << ")" << std::endl;
            return defaultCenter;
        }

        int viewportX, viewportY;
        playerController->GetViewportSize(&viewportX, &viewportY);
        
        SDK::FVector2D center(static_cast<float>(viewportX) / scalingFactor * 0.5f, static_cast<float>(viewportY) / scalingFactor * 0.5f);
        std::cout << "[MATH] GetScreenCenter:" << std::endl;
        std::cout << "[MATH] - Viewport size: " << viewportX << "x" << viewportY << std::endl;
        std::cout << "[MATH] - Scaling factor: " << std::fixed << std::setprecision(2) << scalingFactor << std::endl;
        std::cout << "[MATH] - Screen center: (" << std::fixed << std::setprecision(1) 
                  << center.X << ", " << center.Y << ")" << std::endl;
        
        return center;
    }

    // Enhanced FOV check using screen projection for more accurate targeting
    bool IsWithinScreenFOV(const SDK::FVector& targetPos, const SDK::FVector& cameraPos,
                          SDK::APlayerController* playerController, float fovRadius) {
        if (!playerController) {
            std::cout << "[MATH] IsWithinScreenFOV failed: playerController is null" << std::endl;
            return false;
        }
        
        std::cout << "[MATH] IsWithinScreenFOV check:" << std::endl;
        std::cout << "[MATH] - Target pos: (" << std::fixed << std::setprecision(1) 
                  << targetPos.X << ", " << targetPos.Y << ", " << targetPos.Z << ")" << std::endl;
        std::cout << "[MATH] - FOV radius: " << std::fixed << std::setprecision(1) << fovRadius << " pixels" << std::endl;
        
        SDK::FVector2D targetScreenPos;
        if (!WorldToScreen(targetPos, &targetScreenPos, playerController)) {
            std::cout << "[MATH] - Result: OUTSIDE FOV (not on screen)" << std::endl;
            return false; // Target not on screen
        }
        
        SDK::FVector2D screenCenter = GetScreenCenter(playerController);
        float distanceToCenter = targetScreenPos.GetDistanceTo(screenCenter);
        bool withinFOV = distanceToCenter <= fovRadius;
        
        std::cout << "[MATH] - Target screen pos: (" << std::fixed << std::setprecision(1) 
                  << targetScreenPos.X << ", " << targetScreenPos.Y << ")" << std::endl;
        std::cout << "[MATH] - Distance to center: " << std::fixed << std::setprecision(1) 
                  << distanceToCenter << " pixels" << std::endl;
        std::cout << "[MATH] - Result: " << (withinFOV ? "WITHIN FOV" : "OUTSIDE FOV") << std::endl;
        
        return withinFOV;
    }
    
    // Calculate look-at rotation using UKismetMathLibrary
    SDK::FRotator CalculateLookAtRotation(const SDK::FVector& start, const SDK::FVector& target) {
        SDK::FRotator rotation = SDK::UKismetMathLibrary::FindLookAtRotation(start, target);
        std::cout << "[MATH] CalculateLookAtRotation:" << std::endl;
        std::cout << "[MATH] - Start: (" << std::fixed << std::setprecision(1) 
                  << start.X << ", " << start.Y << ", " << start.Z << ")" << std::endl;
        std::cout << "[MATH] - Target: (" << std::fixed << std::setprecision(1) 
                  << target.X << ", " << target.Y << ", " << target.Z << ")" << std::endl;
        std::cout << "[MATH] - Rotation: (" << std::fixed << std::setprecision(1) 
                  << rotation.Pitch << ", " << rotation.Yaw << ", " << rotation.Roll << ")" << std::endl;
        return rotation;
    }
    
    // Calculate rotation delta using UKismetMathLibrary  
    SDK::FRotator CalculateRotationDelta(const SDK::FRotator& current, const SDK::FRotator& target) {
        SDK::FRotator delta = SDK::UKismetMathLibrary::NormalizedDeltaRotator(target, current);
        std::cout << "[MATH] CalculateRotationDelta:" << std::endl;
        std::cout << "[MATH] - Current: (" << std::fixed << std::setprecision(1) 
                  << current.Pitch << ", " << current.Yaw << ", " << current.Roll << ")" << std::endl;
        std::cout << "[MATH] - Target: (" << std::fixed << std::setprecision(1) 
                  << target.Pitch << ", " << target.Yaw << ", " << target.Roll << ")" << std::endl;
        std::cout << "[MATH] - Delta: (" << std::fixed << std::setprecision(1) 
                  << delta.Pitch << ", " << delta.Yaw << ", " << delta.Roll << ")" << std::endl;
        return delta;
    }
}
