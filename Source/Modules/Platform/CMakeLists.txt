cmake_minimum_required(VERSION 3.25.0)

set (CMAKE_SYSTEM_VERSION 10.0.19041.0)

project(PlatformSystem)

include(GNUInstallDirs)

add_compile_definitions(QUARTZ_PLATFORM_EXPORT)

add_library(${PROJECT_NAME} SHARED 
	"Source/PlatformModule.cpp"
	"Source/Windows/WinApi.cpp"
	"Source/Windows/WinApiPlatform.cpp"
	"Source/Windows/RawInput.cpp"
)

target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_17)

target_include_directories(${PROJECT_NAME} 
	PUBLIC 
		"$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/Include>"
		"$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
	PRIVATE
		"$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/Source>"
		${QUARTZAPP_INCLUDE_PATH}
		${QUARTZLIB_INCLUDE_PATH}
		${QUARTZENGINE_INCLUDE_PATH}
)

target_link_libraries(${PROJECT_NAME}
	QuartzCore
	QuartzApp
	QuartzEngine
)

set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME "Platform")