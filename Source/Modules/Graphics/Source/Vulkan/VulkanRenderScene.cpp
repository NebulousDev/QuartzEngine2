#include "Vulkan/VulkanRenderScene.h"

#include "Log.h"

namespace Quartz
{
	bool FillVulkanRenderSceneSemaphores(VulkanRenderScene& renderScene, uSize multibufferCount)
	{
		if (renderScene.vkDevice == VK_NULL_HANDLE)
		{
			LogError("FillVulkanRenderSceneSemaphores failed: invalid device.");
			return false;
		}

		renderScene.multibufferCount = multibufferCount;
		renderScene.imageAvailableSemaphores.Resize(multibufferCount, VK_NULL_HANDLE);
		renderScene.imageFinishedSemaphores.Resize(multibufferCount, VK_NULL_HANDLE);

		VkSemaphoreCreateInfo semaphoreCreateInfo = {};
		semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
		semaphoreCreateInfo.flags = 0;
		semaphoreCreateInfo.pNext = nullptr;

		for (uSize i = 0; i < multibufferCount; i++)
		{
			if (vkCreateSemaphore(renderScene.vkDevice, &semaphoreCreateInfo,
					VK_NULL_HANDLE, &renderScene.imageAvailableSemaphores[i]) != VK_SUCCESS ||
				vkCreateSemaphore(renderScene.vkDevice, &semaphoreCreateInfo,
					VK_NULL_HANDLE, &renderScene.imageFinishedSemaphores[i]) != VK_SUCCESS)
			{
				LogError("FillVulkanRenderSceneSemaphores failed: vkCreateSemaphore failed.");

				for (uSize j = 0; j < i; j++)
				{
					if (renderScene.imageAvailableSemaphores[j] != VK_NULL_HANDLE)
						vkDestroySemaphore(renderScene.vkDevice, renderScene.imageAvailableSemaphores[j], VK_NULL_HANDLE);
					if (renderScene.imageFinishedSemaphores[j] != VK_NULL_HANDLE)
						vkDestroySemaphore(renderScene.vkDevice, renderScene.imageFinishedSemaphores[j], VK_NULL_HANDLE);
				}

				return false;
			}
		}

		return true;
	}

	bool CreateVulkanRenderScene(EntityWorld& world, VkDevice vkDevice, VkSurfaceKHR vkSurface, uSize multibufferCount, VulkanRenderScene* pVulkanRenderScene)
	{
		pVulkanRenderScene->pWorld = &world;
		pVulkanRenderScene->vkDevice = vkDevice;
		pVulkanRenderScene->vkSurface = vkSurface;

		return FillVulkanRenderSceneSemaphores(*pVulkanRenderScene, multibufferCount);
	}

	void DestroyVulkanRenderScene(VulkanRenderScene* pVulkanRenderScene)
	{
		for (uSize i = 0; i < pVulkanRenderScene->multibufferCount; i++)
		{
			vkDestroySemaphore(pVulkanRenderScene->vkDevice, pVulkanRenderScene->imageAvailableSemaphores[i], VK_NULL_HANDLE);
			vkDestroySemaphore(pVulkanRenderScene->vkDevice, pVulkanRenderScene->imageFinishedSemaphores[i], VK_NULL_HANDLE);
		}
	}
}