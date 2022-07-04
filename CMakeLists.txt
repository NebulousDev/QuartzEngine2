cmake_minimum_required(VERSION 3.20.0)

project(Quartz)

set(QUARTZ_SUPERBUILD ON)

set(QUARTZLIB_INCLUDE_PATH		"${CMAKE_SOURCE_DIR}/Source/Lib/Include")
set(QUARTZAPP_INCLUDE_PATH		"${CMAKE_SOURCE_DIR}/Source/App/Include")
set(QUARTZMATH_INCLUDE_PATH		"${CMAKE_SOURCE_DIR}/Source/Math/Include")
set(QUARTZLOG_INCLUDE_PATH		"${CMAKE_SOURCE_DIR}/Source/Log/Include")
set(QUARTZENGINE_INCLUDE_PATH	"${CMAKE_SOURCE_DIR}/Source/Engine/Include")

set (CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set (CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

add_subdirectory("Source/Lib")
add_subdirectory("Source/Math")
add_subdirectory("Source/Log")
add_subdirectory("Source/App")

add_subdirectory("Source/Engine")
add_subdirectory("Source/Graphics")
