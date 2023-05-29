#pragma once

#include "Entity/World.h"

#include <vulkan/vulkan.h>

namespace Quartz
{
	struct VulkanRenderScene
	{
		EntityWorld*		pWorld;
		VkDevice			vkDevice;
		VkSurfaceKHR		vkSurface;
		uSize				multibufferCount;
		Array<VkSemaphore>	imageAvailableSemaphores;
		Array<VkSemaphore>	imageFinishedSemaphores;
	};

	bool CreateVulkanRenderScene(EntityWorld& world, VkDevice vkDevice, VkSurfaceKHR vkSurface, 
		uSize multibufferCount, VulkanRenderScene* pVulkanRenderScene);

	void DestroyVulkanRenderScene(VulkanRenderScene* pVulkanRenderScene);
}