#include "System/System.h"

#include "Quartz.h"
#include "Entity/World.h"
#include "Log.h"

#include "Platform.h"
#include "Graphics.h"

#include "Vulkan/VulkanGraphics.h"
#include "Vulkan/VulkanSurface.h"
#include "Vulkan/VulkanApiSurface.h"
#include "VulkanRenderer.h"

#include <vulkan/vulkan.h>

namespace Quartz
{
	extern "C"
	{
		using namespace Quartz;

		EntityWorld*	gpWorld;
		Runtime*		gpRuntime;

		bool QUARTZ_API SystemQuery(bool isEditor, Quartz::SystemQueryInfo& systemQuery)
		{
			systemQuery.name = "SandboxModule";
			systemQuery.version = "1.0.0";

			return true;
		}

		bool QUARTZ_API SystemLoad(Log& engineLog, EntityWorld& entityWorld, Runtime& runtime)
		{
			Log::SetGlobalLog(engineLog);

			gpWorld = &entityWorld;
			gpRuntime = &runtime;

			return true;
		}

		void QUARTZ_API SystemUnload()
		{

		}

		void QUARTZ_API SystemPreInit()
		{

		}

		void QUARTZ_API SystemInit()
		{
			LogInfo("Starting Sandbox");

			StartVulkan();

			// TODO: Transition entity system to pointers
			PlatformSingleton& platform = gpWorld->Get<PlatformSingleton>();
			VulkanGraphics&    gfx      = gpWorld->Get<VulkanGraphics>();

			VkSurfaceFormatKHR format = {};
			format.format		= VK_FORMAT_B8G8R8A8_SRGB;
			format.colorSpace	= VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;

			VulkanApiSurfaceInfo apiInfo = {};
			apiInfo.instance			= gfx.vkInstance;
			apiInfo.exclusiveFullscreen = false;
			apiInfo.physicalDevice		= gfx.primaryPhysicalDevice.vkPhysicalDevice;
			apiInfo.surfaceFormat		= format;

			WindowInfo		windowInfo		= { "QuartzEngine 2 - Sandbox", 1280, 720, 100, 100, WINDOW_WINDOWED };
			SurfaceInfo		surfaceInfo		= { SURFACE_API_VULKAN, &apiInfo };
			Window*			pWindow			= platform.pApplication->CreateWindow(windowInfo, surfaceInfo);

			// TEMP
			gfx.pSurface = gfx.pResourceManager->CreateSurface(gfx.vkInstance, &gfx.primaryDevice, (VulkanApiSurface*)pWindow->GetSurface());

			VulkanRenderer* pRenderer = new VulkanRenderer();
			pRenderer->Register(gpRuntime);
			pRenderer->Initialize(&gfx);
		}

	}
}