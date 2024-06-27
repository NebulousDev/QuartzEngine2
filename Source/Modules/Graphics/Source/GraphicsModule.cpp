#include "Module/Module.h"

#include "Engine.h"
#include "EngineAPI.h"
#include "Entity/World.h"
#include "Log.h"

#include "Graphics.h"

#include "Vulkan/VulkanGraphics.h"
#include "Vulkan/VulkanFrameGraph.h"

#ifdef QUARTZAPP_GLEW
#include "gl/glew.h"
#include "gl/GL.h"
#endif

namespace Quartz
{
	Graphics*			gpGraphics;
	VulkanGraphics*		gpVulkanGraphics;

	bool CheckGLAvailable()
	{
#ifdef QUARTZAPP_GLEW
		return glewInit() == GLEW_OK;
#else
		return false;
#endif
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

	void SetupVulkanApiFunctions(GraphicsApiFunctions& functions)
	{
		functions.apiStart = []() -> bool
		{
			return gpVulkanGraphics->Create();
		};

		functions.apiStop = []() -> bool
		{ 
			gpVulkanGraphics->Destroy();
			return true; 
		};

		functions.apiWaitIdle = [](uInt64 time) -> bool
		{
			// Time is ignored in vulkan
			gpVulkanGraphics->WaitIdle();
			return true;
		};

		functions.apiCreateImage = [](
			const GraphicsImageInfo& imageInfo, GraphicsMemoryInfo& outMemoryInfo, void*& pOutNativeImage) -> bool
		{
			VulkanImageInfo vulkanImageInfo = {};
			vulkanImageInfo.width			= imageInfo.width;
			vulkanImageInfo.height			= imageInfo.height;
			vulkanImageInfo.depth			= imageInfo.depth;
			vulkanImageInfo.layers			= imageInfo.layers;
			vulkanImageInfo.mips			= imageInfo.mips;
			//vulkanImageInfo.vkImageType		= VulkanImag

			//VulkanImage* pImage = 
		};
	}

	void QUARTZ_ENGINE_API ModulePreInit() 
	{
		gpVulkanGraphics = &Engine::GetWorld().CreateSingleton<VulkanGraphics>();

		if (CheckVulkanAvailable())
		{
			GraphicsApiFunctions vulkanApiFunctions;
			SetupVulkanApiFunctions(vulkanApiFunctions);

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