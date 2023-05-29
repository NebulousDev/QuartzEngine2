cmake_minimum_required(VERSION 3.20.0)

project(GraphicsSystem)

include(GNUInstallDirs)

add_compile_definitions(QUARTZ_GRAPHICS_EXPORT)

add_library(${PROJECT_NAME} SHARED 
	"Source/GraphicsModule.cpp"
	"Source/Vulkan/VulkanRenderScene.cpp"
    "Source/Vulkan/VulkanGraphics.cpp"
	"Source/Vulkan/VulkanState.cpp"
	"Source/OpenGL/GLGraphics.cpp"
    "Source/VulcanRenderer.cpp"
	"Source/Vulkan/VulkanSwapchian.cpp"
	"Source/Vulkan/VulkanImage.cpp"
	"Source/Vulkan/VulkanResourceManager.cpp"
 "Source/Vulkan/VulkanSurface.cpp")

target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_17)

target_include_directories(${PROJECT_NAME} 
	PUBLIC 
		"$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/Include>"
		"$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
	PRIVATE
		"$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/Source>"
		${QUARTZLIB_INCLUDE_PATH}
		${QUARTZENGINE_INCLUDE_PATH}
)

target_link_libraries(${PROJECT_NAME}
	QuartzCore
	QuartzApp
	QuartzEngine
)

set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME "Graphics")