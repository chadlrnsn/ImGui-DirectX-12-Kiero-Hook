# ImGui DirectX12 Kiero Hook

A DirectX 12 hook implementation using ImGui and Kiero. This project allows you to inject custom ImGui interfaces into DirectX 12 applications.

## Features
- DirectX 12 Hook implementation
- ImGui integration
- Resizable window support
- Fullscreen support
- Minimal performance impact

## Requirements
- Windows 10/11
- Visual Studio 2022
- [DirectX SDK](https://www.microsoft.com/en-us/download/details.aspx?id=6812)
- CMake 3.15 or higher
- Git

## Building the Project

### Using CMake (Recommended)
1. Clone the repository with submodules:
```bash
git clone --recurse-submodules https://github.com/chadlrnsn/ImGui-DirectX-12-Kiero-Hook
cd ImGui-DirectX-12-Kiero-Hook
```

2. Build Release (recommended):
```bash
cmake --preset windows-x64-release
cmake --build build/windows-x64-release --config Release
```

Or Debug:
```bash
cmake --preset windows-x64-debug
cmake --build build/windows-x64-debug --config Debug
```

The DLL will be in:
- Release: `build/windows-x64-release/Release/ImGui_DirectX12_Hook.dll`
- Debug: `build/windows-x64-debug/Debug/ImGui_DirectX12_Hook.dll`

### Using Visual Studio
1. Clone the repository as shown above
2. Open `ImGui-DirectX-12-Kiero-Hook.sln`
3. Select configuration (Debug/Release)
4. Build Solution (Ctrl + B)

## Known Issues
- Debug builds with debug layers enabled might crash
- Minor flickering may occur
- Menu freezes when switching to fullscreen (Alt+Enter) while menu is open

## Implementation Details
This project uses:
- [Kiero](https://github.com/Rebzzel/kiero) for function hooking
- [Dear ImGui](https://github.com/ocornut/imgui) for the user interface
- [MinHook](https://github.com/TsudaKageyu/minhook) for API hooking

## Contributing
Feel free to submit issues and pull requests.

## TODO
- [x] Resize buffers support
- [x] Present hook implementation
- [x] Fullscreen resize buffers
- [ ] Fix fullscreen menu freeze
- [ ] Reduce flickering

[!NOTE]
Only x64 builds are supported due to DirectX 12 limitations.

## License
This project is licensed under the MIT License - see the LICENSE file for details.
