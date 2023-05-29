cmake_minimum_required(VERSION 3.20.0)

project(Quartz)

set(QUARTZ_SUPERBUILD ON)

# Core Lib Paths:
set(QUARTZLIB_INCLUDE_PATH			"${CMAKE_SOURCE_DIR}/Source/Core/Lib/Include")
set(QUARTZAPP_INCLUDE_PATH			"${CMAKE_SOURCE_DIR}/Source/Core/App/Include")
set(QUARTZMATH_INCLUDE_PATH			"${CMAKE_SOURCE_DIR}/Source/Core/Math/Include")
set(QUARTZLOG_INCLUDE_PATH			"${CMAKE_SOURCE_DIR}/Source/Core/Log/Include")

# Module Paths:
set(QUARTZ_PLATFORM_INCLUDE_PATH	"${CMAKE_SOURCE_DIR}/Source/Modules/Platform/Include")
set(QUARTZ_GRAPHICS_INCLUDE_PATH	"${CMAKE_SOURCE_DIR}/Source/Modules/Graphics/Include")

# Engine Path:
set(QUARTZENGINE_INCLUDE_PATH		"${CMAKE_SOURCE_DIR}/Source/Engine/Include")

set (CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set (CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

add_subdirectory("Source/Core/Lib")
add_subdirectory("Source/Core/Math")
add_subdirectory("Source/Core/Log")
add_subdirectory("Source/Core/App")
add_subdirectory("Source/Core")

add_subdirectory("Source/Modules/Platform")
add_subdirectory("Source/Modules/Graphics")
add_subdirectory("Source/Modules/Sandbox")

add_subdirectory("Source/Engine")