#include "Module/Module.h"

#include "Engine.h"
#include "EngineAPI.h"
#include "Entity/World.h"
#include "Log.h"

#include "Graphics.h"

#include "Vulkan/VulkanGraphics.h"

#include "gl/glew.h"
#include "gl/GL.h"

namespace Quartz
{
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
		gpVulkanGraphics = &Engine::GetWorld().CreateSingleton<VulkanGraphics>();

		if (gpVulkanGraphics->ready)
		{
			return true;
		}

		if (gpVulkanGraphics->Create())
		{
			gpGraphics->pInstance = (VulkanGraphics*)gpVulkanGraphics->vkInstance;
			gpGraphics->activeApi = GRAPHICS_API_VULKAN;
		}

		return false;
	}

	bool StopVulkan()
	{
		gpVulkanGraphics->Destroy();
		Engine::GetWorld().DestroySingleton<VulkanGraphics>();
		return true;
	}
}

extern "C"
{
	using namespace Quartz;

	bool QUARTZ_ENGINE_API ModuleQuery(bool isEditor, Quartz::ModuleQueryInfo& moduleQuery)
	{
		moduleQuery.name = "GraphicsModule";
		moduleQuery.version = "1.0.0";

		return true;
	}

	bool QUARTZ_ENGINE_API ModuleLoad(Log& engineLog, Engine& engine)
	{
		Log::SetInstance(engineLog);
		Engine::SetInstance(engine);
		return true;
	}

	void QUARTZ_ENGINE_API ModuleUnload()
	{
		gpVulkanGraphics->Destroy();
	}

	void QUARTZ_ENGINE_API ModulePreInit()
	{
		gpGraphics = &Engine::GetWorld().CreateSingleton<Graphics>();

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

	void QUARTZ_ENGINE_API ModuleInit()
	{
		
	}
}