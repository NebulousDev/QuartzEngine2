cmake_minimum_required(VERSION 3.25.0)

project(QuartzCore)

include(GNUInstallDirs)

add_library(${PROJECT_NAME} SHARED "Core.cpp" "Math/Include/Math/Transform.h")

target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_17)

target_link_libraries(${PROJECT_NAME}
	PUBLIC
		QuartzLib
		QuartzMath
		QuartzLog
)

target_include_directories(${PROJECT_NAME} 
	PUBLIC
		${QUARTZLIB_INCLUDE_PATH}
		${QUARTZMATH_INCLUDE_PATH}
		${QUARTZLOG_INCLUDE_PATH}
)

set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME "Core")