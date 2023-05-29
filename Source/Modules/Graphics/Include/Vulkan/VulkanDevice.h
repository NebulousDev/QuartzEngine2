#pragma once

#include <vulkan/vulkan.h>

namespace Quartz
{
	struct VulkanPhysicalDevice;

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