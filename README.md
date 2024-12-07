# ImGui-DirectX-12-Kiero-Hook

Universal ImGui implementation through DirectX 12 Hook using kiero. This project allows you to inject ImGui into DirectX 12 applications.

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

## Building the Project

### Option 1: Using CMake (Recommended)
1. Clone the repository with submodules:
```bash
git clone --recurse-submodules https://github.com/chadlrnsn/ImGui-DirectX-12-Kiero-Hook
cd ImGui-DirectX-12-Kiero-Hook
```

2. Build using CMake:
```bash
# Debug build
cmake --preset windows-x64-debug
cmake --build build/windows-x64-debug --config Debug

# Release build
cmake --preset windows-x64-release
cmake --build build/windows-x64-release --config Release
```

The compiled DLL will be in `build/windows-x64-[debug|release]/[Debug|Release]/ImGui_DirectX12_Hook.dll`

### Option 2: Using Visual Studio Solution
1. Clone the repository as shown above
2. Open `ImGui-DirectX-12-Kiero-Hook.sln`
3. Build the solution in Debug or Release configuration

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

## License
This project is licensed under the MIT License - see the LICENSE file for details.
