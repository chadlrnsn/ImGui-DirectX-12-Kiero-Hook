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

        const char* Input::GetKeyName(BYTE vkCode) {
            switch (vkCode) {
                // Mouse buttons
                case VK_LBUTTON: return "Left Mouse";
                case VK_RBUTTON: return "Right Mouse";
                case VK_MBUTTON: return "Middle Mouse";
                case VK_XBUTTON1: return "Mouse4";
                case VK_XBUTTON2: return "Mouse5";

                // Function keys
                case VK_F1: return "F1";
                case VK_F2: return "F2";
                case VK_F3: return "F3";
                case VK_F4: return "F4";
                case VK_F5: return "F5";
                case VK_F6: return "F6";
                case VK_F7: return "F7";
                case VK_F8: return "F8";
                case VK_F9: return "F9";
                case VK_F10: return "F10";
                case VK_F11: return "F11";
                case VK_F12: return "F12";

                // Number keys
                case '0': return "0";
                case '1': return "1";
                case '2': return "2";
                case '3': return "3";
                case '4': return "4";
                case '5': return "5";
                case '6': return "6";
                case '7': return "7";
                case '8': return "8";
                case '9': return "9";

                // Letter keys
                case 'A': return "A";
                case 'B': return "B";
                case 'C': return "C";
                case 'D': return "D";
                case 'E': return "E";
                case 'F': return "F";
                case 'G': return "G";
                case 'H': return "H";
                case 'I': return "I";
                case 'J': return "J";
                case 'K': return "K";
                case 'L': return "L";
                case 'M': return "M";
                case 'N': return "N";
                case 'O': return "O";
                case 'P': return "P";
                case 'Q': return "Q";
                case 'R': return "R";
                case 'S': return "S";
                case 'T': return "T";
                case 'U': return "U";
                case 'V': return "V";
                case 'W': return "W";
                case 'X': return "X";
                case 'Y': return "Y";
                case 'Z': return "Z";

                // Special keys
                case VK_SPACE: return "Space";
                case VK_RETURN: return "Enter";
                case VK_ESCAPE: return "Escape";
                case VK_TAB: return "Tab";
                case VK_SHIFT: return "Shift";
                case VK_CONTROL: return "Ctrl";
                case VK_MENU: return "Alt";
                case VK_CAPITAL: return "Caps Lock";
                case VK_INSERT: return "Insert";
                case VK_DELETE: return "Delete";
                case VK_HOME: return "Home";
                case VK_END: return "End";
                case VK_PRIOR: return "Page Up";
                case VK_NEXT: return "Page Down";
                case VK_UP: return "Up Arrow";
                case VK_DOWN: return "Down Arrow";
                case VK_LEFT: return "Left Arrow";
                case VK_RIGHT: return "Right Arrow";

                default: return "Unknown";
            }
        }

        BYTE Input::CaptureNextKeyPress() {
            // Check all possible keys for a press
            for (int vk = 1; vk < 256; vk++) {
                if (vk == VK_LBUTTON || vk == VK_RBUTTON) continue; // Skip left/right mouse to avoid UI conflicts

                if (IsKeyPressed(vk)) {
                    return static_cast<BYTE>(vk);
                }
            }
            return 0; // No key pressed
        }

    } // namespace Utils
} // namespace Cheat
