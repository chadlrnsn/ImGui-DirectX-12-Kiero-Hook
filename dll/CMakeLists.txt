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


set(DLL_NAME "${PROJECT_NAME}-ImGui")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(${DLL_NAME} SHARED 
    src/dllmain.cpp
    ${CMAKE_SOURCE_DIR}/vendor/kiero/kiero.cpp
    src/hooks/d3d12hook.cpp
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

target_include_directories(${DLL_NAME} PRIVATE 
    ${imgui_SOURCE_DIR}
    ${imgui_SOURCE_DIR}/backends
    ${minhook_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/vendor/kiero
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)
