#include "Module/Module.h"

#include "Engine.h"
#include "EngineAPI.h"
#include "Entity/World.h"
#include "Log.h"

#include "Graphics.h"

#include "Vulkan/VulkanGraphics.h"
#include "Vulkan/VulkanFrameGraph.h"

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
}

extern "C"
{
	using namespace Quartz;

	bool QUARTZ_ENGINE_API ModuleQuery(bool isEditor, Quartz::ModuleQueryInfo& moduleQuery)
	{
		moduleQuery.name	= "GraphicsModule";
		moduleQuery.version = "1.0.0";
		moduleQuery.type	= MODULE_TYPE_GRAPHICS;

		return true;
	}

	bool QUARTZ_ENGINE_API ModuleLoad(Log& engineLog, Engine& engine)
	{
		Log::SetInstance(engineLog);
		Engine::SetInstance(engine);
		return true;
	}

	void QUARTZ_ENGINE_API ModulePreInit() 
	{
		gpVulkanGraphics = &Engine::GetWorld().CreateSingleton<VulkanGraphics>();

		if (CheckVulkanAvailable())
		{
			GraphicsApiFunctions vulkanApiFunctions;
			vulkanApiFunctions.apiStartFunc = []() -> bool 
			{
				return gpVulkanGraphics->Create();
			};

			vulkanApiFunctions.apiStopFunc = []() -> bool
			{ 
				gpVulkanGraphics->Destroy(); return true; 
			};

			vulkanApiFunctions.apiWaitIdleFunc = []() -> bool
			{
				gpVulkanGraphics->WaitIdle(); return true;
			};

			VulkanFrameGraph* pVulkanFrameGraph = new VulkanFrameGraph();

			GraphicsApiInfo vulkanApiInfo = {};
			vulkanApiInfo.fullName		= "Vulkan";
			vulkanApiInfo.version		= "1.3.0";
			vulkanApiInfo.apiFunctions	= vulkanApiFunctions;
			vulkanApiInfo.pFrameGraph	= pVulkanFrameGraph;
			vulkanApiInfo.pNativeApi	= gpVulkanGraphics;

			Engine::GetGraphics().RegisterApi("vulkan", vulkanApiInfo);
		}
	}

	void QUARTZ_ENGINE_API ModuleInit()
	{
		
	}

	void QUARTZ_ENGINE_API ModuleUnload()
	{
		if (gpVulkanGraphics)
		{
			gpVulkanGraphics->Destroy();
		}
	}
}