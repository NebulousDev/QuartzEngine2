cmake_minimum_required(VERSION 3.20.0)

project(SandboxSystem)

include(GNUInstallDirs)

add_library(${PROJECT_NAME} SHARED 
	"Source/SandboxModule.cpp"
	"Source/Physics.cpp"
	"Source/Collisions.cpp"
	"Source/GJK.cpp"
	"Source/Simplex.cpp"
	"Source/Inertia.cpp"
	"Source/Collision.cpp" "Include/PhysicsTypes.h")

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

		${QUARTZ_PLATFORM_INCLUDE_PATH}
		${QUARTZ_GRAPHICS_INCLUDE_PATH}
)

target_link_libraries(${PROJECT_NAME}
	QuartzCore
	QuartzApp
	QuartzEngine
	GraphicsSystem
)

set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME "Sandbox")