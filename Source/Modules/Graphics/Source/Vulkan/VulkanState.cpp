#include "Vulkan/VulkanState.h"

namespace Quartz
{
	bool CreateVulkanState(const VulkanGraphics* pGraphics, VulkanState* pState)
	{
		return true;
	}

	void DestroyVulkanState(VulkanState* pState)
	{

	}

	void AddRenderScene(VulkanState* pState, const VulkanRenderScene& renderScene)
	{
		pState->renderScenes.PushBack(renderScene);
	}
}