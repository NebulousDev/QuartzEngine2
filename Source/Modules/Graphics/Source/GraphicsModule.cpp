#include "System/System.h"

#include "EngineAPI.h"
#include "Entity/World.h"
#include "Log.h"

#include "Graphics.h"

#include "Vulkan/VulkanGraphics.h"
#include "Vulkan/VulkanState.h"

#include "gl/glew.h"
#include "gl/GL.h"

namespace Quartz
{
	EntityWorld*		gpEntityWorld;
	Runtime*			gpRuntime;
	Graphics*			gpGraphics;
	VulkanGraphics*		gpVulkanGraphics;

	bool CheckGLAvailable()
	{
		return glewInit() == GLEW_OK;
	}

	bool CheckVulkanAvailable()
	{
		return gpVulkanGraphics->TestVersion(VK_API_VERSION_1_2);
	}

	bool CheckD3D12Available()
	{
		return false;
	}

	bool InitOpenGL()
	{
		return glewInit() == GLEW_OK;
	}

	bool StartVulkan()
	{
		gpVulkanGraphics = &gpEntityWorld->CreateSingleton<VulkanGraphics>();

		if (gpVulkanGraphics->ready)
		{
			return true;
		}

		if (gpVulkanGraphics->Create())
		{
			gpGraphics->pInstance = (VulkanGraphics*)gpVulkanGraphics->vkInstance;
			gpGraphics->activeApi = GRAPHICS_API_VULKAN;
		}

		gpVulkanGraphics->pEntityWorld = gpEntityWorld; // @TODO: find a better solution

		return false;
	}

	bool StopVulkan()
	{
		gpVulkanGraphics->Destroy();
		gpEntityWorld->DestroySingleton<VulkanGraphics>();
		return true;
	}
}

extern "C"
{
	using namespace Quartz;

	bool QUARTZ_ENGINE_API SystemQuery(bool isEditor, Quartz::SystemQueryInfo& systemQuery)
	{
		systemQuery.name = "GraphicsModule";
		systemQuery.version = "1.0.0";

		return true;
	}

	bool QUARTZ_ENGINE_API SystemLoad(Log& engineLog, EntityWorld& entityWorld, Runtime& runtime)
	{
		Log::SetGlobalLog(engineLog);
		gpEntityWorld = &entityWorld;
		gpRuntime = &runtime;
		return true;
	}

	void QUARTZ_ENGINE_API SystemUnload()
	{
		gpVulkanGraphics->Destroy();
	}

	void QUARTZ_ENGINE_API SystemPreInit()
	{
		gpGraphics = &gpEntityWorld->CreateSingleton<Graphics>();

		/* Check API Availability */

		if (CheckGLAvailable())
		{
			gpGraphics->available.openGL = true;
		}
		
		if (CheckVulkanAvailable())
		{
			gpGraphics->available.vulkan = true;
		}

		if (CheckD3D12Available())
		{
			gpGraphics->available.d3d12 = true;
		}
	}

	void QUARTZ_ENGINE_API SystemInit()
	{
		
	}
}