cmake_minimum_required(VERSION 3.15)
project(ImGui_DirectX12_Hook)

# Указываем все исходные файлы
set(SOURCE_FILES
    # Основной файл примера DirectX12
    ImGui-DirectX-12-Kiero-Hook/src/dllmain.cpp
    ImGui-DirectX-12-Kiero-Hook/src/d3d12hook.cpp

    # Kiero
    ImGui-DirectX-12-Kiero-Hook/vendor/kiero/kiero.cpp

    # Файлы ImGui
    ImGui-DirectX-12-Kiero-Hook/vendor/imgui/imgui.cpp
    ImGui-DirectX-12-Kiero-Hook/vendor/imgui/imgui_demo.cpp
    ImGui-DirectX-12-Kiero-Hook/vendor/imgui/imgui_draw.cpp
    ImGui-DirectX-12-Kiero-Hook/vendor/imgui/imgui_tables.cpp
    ImGui-DirectX-12-Kiero-Hook/vendor/imgui/imgui_widgets.cpp
    ImGui-DirectX-12-Kiero-Hook/vendor/imgui/backends/imgui_impl_dx12.cpp
    ImGui-DirectX-12-Kiero-Hook/vendor/imgui/backends/imgui_impl_win32.cpp
)

# Указываем пути для включаемых файлов
include_directories(
    ImGui-DirectX-12-Kiero-Hook/src
    ImGui-DirectX-12-Kiero-Hook/vendor
    # minhook
    ImGui-DirectX-12-Kiero-Hook/vendor/minhook/include
    # kiero
    ImGui-DirectX-12-Kiero-Hook/vendor/kiero
    # imgui
    ImGui-DirectX-12-Kiero-Hook/vendor/imgui
    ImGui-DirectX-12-Kiero-Hook/vendor/imgui/backends
)

# Создаем DLL
add_library(${PROJECT_NAME} SHARED ${SOURCE_FILES})

# Добавляем MinHook
add_subdirectory(ImGui-DirectX-12-Kiero-Hook/vendor/minhook)

# Линкуем необходимые библиотеки
target_link_libraries(${PROJECT_NAME} PRIVATE
    minhook
    d3d12
    dxgi
)