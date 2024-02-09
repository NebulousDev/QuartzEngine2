#pragma once

#include "Types/Types.h"
#include "Types/Array.h"
#include <vulkan/vulkan.h>

namespace Quartz
{
	struct VulkanDescriptorPoolInfo
	{
		Array<VkDescriptorPoolSize>	sizes;
		uSize						maxSets;
	};

	struct VulkanDescriptorPool
	{
		VkDescriptorPool			vkDescriptorPool;
		Array<VkDescriptorPoolSize>	sizes;
		uSize						maxSets;
	};
}