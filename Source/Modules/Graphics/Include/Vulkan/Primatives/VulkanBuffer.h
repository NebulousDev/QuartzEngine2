#pragma once

#include "Types/Types.h"
#include "VulkanDevice.h"

namespace Quartz
{
	struct VulkanBufferInfo
	{
		uSize					sizeBytes;
		VkBufferUsageFlags		vkUsageFlags;
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

		inline bool operator==(const VulkanBuffer& value) { return vkBuffer == value.vkBuffer; }
	};
}