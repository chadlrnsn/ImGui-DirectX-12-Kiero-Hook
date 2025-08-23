#include "Input.h"

namespace Cheat {
    namespace Utils {
        
        std::unordered_map<int, bool> Input::s_previousKeyStates;
        std::unordered_map<int, bool> Input::s_currentKeyStates;
        
        bool Input::IsKeyPressed(int vkCode) {
            bool currentState = GetKeyState(vkCode);
            bool previousState = s_previousKeyStates[vkCode];
            
            // Update states
            s_previousKeyStates[vkCode] = currentState;
            s_currentKeyStates[vkCode] = currentState;
            
            // Key is pressed if it's currently down but wasn't down before
            return currentState && !previousState;
        }
        
        bool Input::IsKeyDown(int vkCode) {
            return GetKeyState(vkCode);
        }
        
        bool Input::IsKeyUp(int vkCode) {
            return !GetKeyState(vkCode);
        }
        
        void Input::Update() {
            // Update previous states for all tracked keys
            for (auto& pair : s_currentKeyStates) {
                s_previousKeyStates[pair.first] = pair.second;
                s_currentKeyStates[pair.first] = GetKeyState(pair.first);
            }
        }
        
        bool Input::GetKeyState(int vkCode) {
            return (GetAsyncKeyState(vkCode) & 0x8000) != 0;
        }
        
    } // namespace Utils
} // namespace Cheat
