include(FetchContent)

FetchContent_Declare(
    imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG v1.91.7
)
FetchContent_MakeAvailable(imgui)

set(IMGUI_SOURCES
    ${imgui_SOURCE_DIR}/imgui.cpp
    ${imgui_SOURCE_DIR}/imgui_demo.cpp
    ${imgui_SOURCE_DIR}/imgui_draw.cpp
    ${imgui_SOURCE_DIR}/imgui_tables.cpp
    ${imgui_SOURCE_DIR}/imgui_widgets.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_dx12.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_win32.cpp
)

add_library(imgui STATIC ${IMGUI_SOURCES})
target_include_directories(imgui PUBLIC
    ${imgui_SOURCE_DIR}
    ${imgui_SOURCE_DIR}/backends
)

target_include_directories(imgui PUBLIC
    ${CMAKE_CURRENT_BINARY_DIR}
)


set(DLL_NAME "Abyssus")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(${DLL_NAME} SHARED
    src/dllmain.cpp
    ${CMAKE_SOURCE_DIR}/vendor/kiero/kiero.cpp
    src/hooks/d3d12hook.cpp
    src/dev/Console.cpp

    # Cheat system core
    src/cheats/Core/CheatMain.cpp
    src/cheats/Core/Config.cpp

    # Services (cheat runtime)
    src/cheats/Services/GameServices.cpp
    src/cheats/Services/PlayerEffectsService.cpp
    src/cheats/Services/AimbotService.cpp
    src/cheats/Services/TargetingService.cpp
    src/cheats/Services/BoneService.cpp
    src/cheats/Services/WeaponService.cpp
    src/cheats/Services/UnlockService.cpp
    src/cheats/Services/SaveService.cpp
    src/cheats/Services/ResourceService.cpp

    # GUI
    src/gui/CheatMenu.cpp
        src/gui/Tabs/AimbotTab.cpp
        src/gui/Tabs/WeaponsTab.cpp
        src/gui/Tabs/GameplayTab.cpp
        src/gui/Tabs/OptionsTab.cpp
        src/gui/Tabs/UnlocksTab.cpp
        src/gui/Tabs/DebugTab.cpp


    # Cheat system utilities
    src/cheats/Utils/Console.cpp
    src/cheats/Utils/Input.cpp

    # Existing cheat system components
    src/cheats/Aimbot/AimbotController.cpp
    src/cheats/Aimbot/MathUtils.cpp
    src/cheats/Aimbot/TargetSelector.cpp
    src/cheats/Analysis/BoneAnalyzer.cpp

    # SDK implementation files
    src/cheats/SDK/Basic.cpp
    src/cheats/SDK/BP_EngineRifle_Script_Functions.cpp
    src/cheats/SDK/CoreUObject_functions.cpp
    src/cheats/SDK/Engine_functions.cpp
    src/cheats/SDK/RGame_functions.cpp
)

set_output_directories(${DLL_NAME})
add_unicode_definitions(${DLL_NAME})

target_link_libraries(${DLL_NAME} PRIVATE
    imgui
    minhook
    fmt::fmt
    d3d12.lib
    dxgi.lib
)

# Generate PDB files in release mode
if(CMAKE_BUILD_TYPE STREQUAL "Release")
    target_link_options(${DLL_NAME} PRIVATE "/DEBUG")
endif()

target_include_directories(${DLL_NAME} PRIVATE
    ${imgui_SOURCE_DIR}
    ${imgui_SOURCE_DIR}/backends
    ${minhook_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/vendor/kiero
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/src/framework
)

#pch
target_precompile_headers(${DLL_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/framework/stdafx.h
)

