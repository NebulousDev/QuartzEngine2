#pragma once

#include <vulkan/vulkan.h>
#include "Types/Array.h"

namespace Quartz
{
	struct VulkanPhysicalDevice
	{
		VkPhysicalDevice					vkPhysicalDevice;
		VkPhysicalDeviceProperties			vkProperties;
		VkPhysicalDeviceFeatures			vkFeatures;
		VkPhysicalDeviceMemoryProperties	vkMemoryProperties;
		Array<VkQueueFamilyProperties>		vkQueueFamilyProperties;

		struct
		{
			uInt32							graphics;
			uInt32							compute;
			uInt32							transfer;
			uInt32							present;
		}
		primaryQueueFamilyIndices;
	};
}