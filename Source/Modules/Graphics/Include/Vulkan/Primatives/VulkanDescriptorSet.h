#pragma once

#include "VulkanDescriptorPool.h"
#include <vulkan/vulkan.h>

namespace Quartz
{
	struct VulkanDescriptorSetLayout;

	struct VulkanDescriptorSetAllocationInfo
	{
		VulkanDescriptorPool*				pDescriptorPool;
		Array<VulkanDescriptorSetLayout*>	setLayouts;
	};

	struct VulkanDescriptorSet
	{
		VkDescriptorSet			vkDescriptorSet;
		VulkanDescriptorPool*	pDescriptorPool;
	};
}