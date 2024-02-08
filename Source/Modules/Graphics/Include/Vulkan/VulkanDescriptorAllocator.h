#pragma once

#include "VulkanDescriptorSet.h"
#include "Types/Map.h"

namespace Quartz
{
	class VulkanDescriptorAllocator
	{
	private:
		Map<VulkanDescriptorPool*, VulkanDescriptorSet*> mPoolSetMap;

	public:


	};
}