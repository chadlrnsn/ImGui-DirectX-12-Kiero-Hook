file(GLOB_RECURSE SOURCES "src/*.cpp")
add_executable(${PROJECT_NAME} WIN32 ${SOURCES}) # WIN32 for wWinMain
target_include_directories(${PROJECT_NAME} PRIVATE src)
target_link_libraries(${PROJECT_NAME} PRIVATE fmt::fmt)

# Применяем общие настройки
set_output_directories(${PROJECT_NAME})
add_unicode_definitions(${PROJECT_NAME})

target_link_libraries(${PROJECT_NAME} PRIVATE 
    d3d12.lib
    dxgi.lib
)

target_precompile_headers(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/stdafx.h
)