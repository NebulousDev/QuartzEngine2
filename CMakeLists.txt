cmake_minimum_required(VERSION 3.20.0)

project(Quartz)

set(QUARTZ_SUPERBUILD ON)

# Core Lib Paths:
set(QUARTZLIB_INCLUDE_PATH			"${CMAKE_SOURCE_DIR}/Source/Core/Lib/Include")
set(QUARTZAPP_INCLUDE_PATH			"${CMAKE_SOURCE_DIR}/Source/Core/App/Include")
set(QUARTZMATH_INCLUDE_PATH			"${CMAKE_SOURCE_DIR}/Source/Core/Math/Include")
set(QUARTZLOG_INCLUDE_PATH			"${CMAKE_SOURCE_DIR}/Source/Core/Log/Include")

set(QUARTZLIB_SOURCE_PATH			"${CMAKE_SOURCE_DIR}/Source/Core/Lib/Source")
set(QUARTZAPP_SOURCE_PATH			"${CMAKE_SOURCE_DIR}/Source/Core/App/Source")
set(QUARTZMATH_SOURCE_PATH			"${CMAKE_SOURCE_DIR}/Source/Core/Math/Source")
set(QUARTZLOG_SOURCE_PATH			"${CMAKE_SOURCE_DIR}/Source/Core/Log/Source")

# Module Paths:
set(QUARTZ_PLATFORM_PATH			"${CMAKE_SOURCE_DIR}/Source/Modules/Platform")
set(QUARTZ_GRAPHICS_PATH			"${CMAKE_SOURCE_DIR}/Source/Modules/Graphics")

set(QUARTZ_PLATFORM_INCLUDE_PATH	"${CMAKE_SOURCE_DIR}/Source/Modules/Platform/Include")
set(QUARTZ_GRAPHICS_INCLUDE_PATH	"${CMAKE_SOURCE_DIR}/Source/Modules/Graphics/Include")

set(QUARTZ_PLATFORM_SOURCE_PATH		"${CMAKE_SOURCE_DIR}/Source/Modules/Platform/Source")
set(QUARTZ_GRAPHICS_SOURCE_PATH		"${CMAKE_SOURCE_DIR}/Source/Modules/Graphics/Source")


# Engine Path:
set(QUARTZ_ENGINE_INCLUDE_PATH		"${CMAKE_SOURCE_DIR}/Source/Engine/Source")
set(QUARTZ_ENGINE_SOURCE_PATH		"${CMAKE_SOURCE_DIR}/Source/Engine/Source")

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