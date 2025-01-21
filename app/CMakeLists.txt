# Основное приложение DirectX 12
add_executable(${PROJECT_NAME} src/main.cpp) # WIN32 for wWinMain

# Применяем общие настройки
set_output_directories(${PROJECT_NAME})
add_unicode_definitions(${PROJECT_NAME})
add_directx_dependencies(${PROJECT_NAME}) 