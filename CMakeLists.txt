cmake_minimum_required(VERSION 3.24)
project(DirectX12-Example VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/.bin")

include(cmake/common.cmake)
include(cmake/dependencies.cmake)

add_subdirectory(dll)

if(BUILD_INSTALLER)
    include(cmake/install.cmake)
endif()

