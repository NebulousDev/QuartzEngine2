#pragma once

#include "Types/Types.h"
#include "Types/Array.h"
#include <vulkan/vulkan.h>

namespace Quartz
{
	struct VulkanDesctiptorSetLayoutBinding
	{
		VkDescriptorSetLayoutBinding	vkBinding;
		uSize							sizeBytes;
	};

	struct VulkanDesctiptorSetLayoutInfo
	{
		uSize									set;
		Array<VulkanDesctiptorSetLayoutBinding> setBindings;
	};

	struct VulkanDescriptorSetLayout
	{
		VkDescriptorSetLayout						vkDescriptorSetLayout;
		uInt32										set;
		Array<VulkanDesctiptorSetLayoutBinding, 16> setBindings;
		uInt32										sizeBytes;
	};
}