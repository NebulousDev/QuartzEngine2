#pragma once

#include "VulkanDevice.h"
#include "VulkanRenderpass.h"
#include "VulkanFrameBuffer.h"
#include "VulkanPipeline.h"
#include "VulkanBuffer.h"

namespace Quartz
{
	struct VulkanCommandPoolInfo
	{
		uInt32						queueFamilyIndex;
		VkCommandPoolCreateFlags	vkCommandPoolCreateFlags;
	};

	struct VulkanCommandPool
	{
		VkCommandPool				vkCommandPool;
		VulkanDevice*				pDevice;
		uInt32						queueFamilyIndex;
		VkCommandPoolCreateFlags	vkCommandPoolCreateFlags;
	};

	struct VulkanCommandBuffer
	{
		VkCommandBuffer		vkCommandBuffer;
		VulkanDevice*		pDevice;
		VulkanCommandPool*	pCommandPool;
	};
}