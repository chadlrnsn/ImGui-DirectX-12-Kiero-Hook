cmake_minimum_required(VERSION 3.24)
project(DirectX12-Example)

# Общие настройки
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Установка глобальной директории для выходных файлов
set(OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/.bin")

# Подключаем общие настройки и функции
include(cmake/common.cmake)
include(cmake/dependencies.cmake)

# Подключаем подпроекты
add_subdirectory(app)
add_subdirectory(dll)

