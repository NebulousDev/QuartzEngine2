#pragma once

#include "Types/Types.h"
#include "VulkanDevice.h"

namespace Quartz
{
	struct VulkanBufferInfo
	{
		uSize					sizeBytes;
		VkBufferUsageFlags		vkBufferUsage;
		VkMemoryPropertyFlags	vkMemoryProperties;
	};

	struct VulkanBuffer
	{
		VkBuffer				vkBuffer;
		VulkanDevice*			pDevice;
		uSize					sizeBytes;
		VkDeviceMemory			vkMemory;
		VkBufferUsageFlags		vkUsage;
		VkMemoryPropertyFlags	vkMemoryProperties;
	};
}