#pragma once

#include <vulkan/vulkan.h>
#include "VulkanPhysicalDevice.h"

namespace Quartz
{
	struct VulkanDevice
	{
		VkDevice				vkDevice;
		VulkanPhysicalDevice*	pPhysicalDevice;

		struct
		{
			VkQueue				graphics;
			VkQueue				compute;
			VkQueue				transfer;
			VkQueue				present;
		}
		queues;
	};
}