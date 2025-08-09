# Имя DLL
set(DLL_NAME "${PROJECT_NAME}-ImGui")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Создаем DLL
add_library(${DLL_NAME} SHARED 
    src/dllmain.cpp
    ${CMAKE_SOURCE_DIR}/vendor/kiero/kiero.cpp
    src/hooks/d3d12hook.cpp
)

set_output_directories(${DLL_NAME})
add_unicode_definitions(${DLL_NAME})
add_directx_dependencies(${DLL_NAME})

target_link_libraries(${DLL_NAME} PRIVATE 
    imgui::imgui
    minhook::MinHook
    fmt::fmt
)

# Добавляем пути для включения
target_include_directories(${DLL_NAME} PRIVATE 
    ${imgui_SOURCE_DIR}
    ${imgui_SOURCE_DIR}/backends
    ${minhook_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/vendor/kiero
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# Копируем DLL в директорию с исполняемым файлом
add_custom_command(TARGET ${DLL_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy 
        $<TARGET_FILE:${DLL_NAME}>
        ${OUTPUT_DIRECTORY}/$<CONFIG>/${DLL_NAME}.dll
) 