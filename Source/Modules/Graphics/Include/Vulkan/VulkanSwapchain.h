#pragma once

#include "Vulkan/VulkanImage.h"
#include "Vulkan/VulkanDevice.h"

#include "Types/Array.h"

namespace Quartz
{
	struct VulkanSwapchain
	{
		VkSwapchainKHR			vkSwapchain;
		VulkanDevice*			pDevice;
		uInt32					backbufferCount;

		Array<VulkanImage*>		images;
		Array<VulkanImageView*>	imageViews;
		Array<VkSemaphore>		imageAvailableSemaphores;
		Array<VkSemaphore>		imageCompleteSemaphores;
		Array<VkFence>			imageFences;
	};
}