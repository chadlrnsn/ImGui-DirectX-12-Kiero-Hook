@echo off
cd .\ImGui-DirectX-12-Kiero-Hook\vendor\imgui\examples\example_win32_directx12
if not exist "Debug" (
    echo FIRST COMPILE DX12 EXAMPLE WINDOW!!!
    pause
    exit /b 1
) else (
    start ./Debug/example_win32_directx12.exe
)
