cmake_minimum_required(VERSION 3.14)
include(FetchContent)

project(MyProject LANGUAGES C)

set(BINARY_DIR ${CMAKE_CURRENT_SOURCE_DIR}/bin)
set(SOURCE_DIR ${CMAKE_SOURCE_DIR}/src)
set(INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)
set(CMAKE_MODULES_DIR ${CMAKE_CURRENT_SOURCE_DIR}/cmake-modules)

#include_directories(${BINARY_DIR})
include_directories(${SOURCE_DIR})
include_directories(${INCLUDE_DIR})
include_directories(${CMAKE_MODULES_DIR})

add_subdirectory(${SOURCE_DIR})