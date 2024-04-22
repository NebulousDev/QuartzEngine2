cmake_minimum_required(VERSION 3.25.0)

project(GraphicsSystem)

include(GNUInstallDirs)

add_compile_definitions(QUARTZ_GRAPHICS_EXPORT)

file(GLOB imgui_sources 
	"${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/imgui/*.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/imgui/backends/imgui_impl_vulkan.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/imgui/backends/imgui_impl_glfw.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/imgui/backends/imgui_impl_win32.cpp"
)

add_library(${PROJECT_NAME} SHARED
	${imgui_sources}
	"Source/GraphicsModule.cpp"
	"Source/Vulkan/VulkanBufferCache.cpp"
    "Source/Vulkan/VulkanGraphics.cpp"
	"Source/OpenGL/GLGraphics.cpp"
    "Source/Vulkan/VulkanRenderer.cpp"
	"Source/Vulkan/VulkanSwapchianTimer.cpp"
	"Source/Vulkan/VulkanResourceManager.cpp"
	"Source/Vulkan/SPIRV/SPIRV.cpp"
	"Source/Vulkan/VulkanBufferWriter.cpp"
	"Source/Vulkan/VulkanCommandRecorder.cpp"
	"Source/Component/TransformComponent.cpp"
	"Source/Component/MeshComponent.cpp"
	"Source/Vulkan/VulkanMultiBuffer.cpp"
	"Source/Component/MaterialComponent.cpp"
	"Source/Vulkan/VulkanPipelineCache.cpp"
	"Source/Vulkan/VulkanShaderCache.cpp"
	"Source/Component/CameraComponent.cpp"
	
	"Source/Vulkan/Renderers/VulkanTerrainRenderer.cpp"
	"Source/Vulkan/Renderers/VulkanSkyRenderer.cpp"
	"Source/Vulkan/Renderers/VulkanSceneRenderer.cpp"
	"Source/Vulkan/Renderers/VulkanImGuiRenderer.cpp"
 )

target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_17)

target_include_directories(${PROJECT_NAME} 
	PUBLIC 
		"$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/Include>"
		"$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/imgui>"
		"$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
	PRIVATE
		"$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/Source>"
		"$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/imgui>"
		${QUARTZLIB_INCLUDE_PATH}
		${QUARTZENGINE_INCLUDE_PATH}
)

target_link_libraries(${PROJECT_NAME}
	QuartzCore
	QuartzApp
	QuartzEngine
)

set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME "Graphics")