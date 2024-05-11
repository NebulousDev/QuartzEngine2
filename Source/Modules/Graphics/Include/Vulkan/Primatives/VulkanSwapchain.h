#pragma once

#include "VulkanImage.h"
#include "VulkanDevice.h"

#include "Types/Array.h"

namespace Quartz
{
	struct VulkanSwapchain
	{
		VkSwapchainKHR			vkSwapchain;
		VulkanDevice*			pDevice;
		uInt32					backbufferCount;

		Array<VulkanImage*, 8>		images;
		Array<VulkanImageView*, 8>	imageViews;
		Array<VkSemaphore, 8>		imageAvailableSemaphores;
		Array<VkSemaphore, 8>		imageCompleteSemaphores;
		Array<VkFence, 8>			imageFences;
	};
}