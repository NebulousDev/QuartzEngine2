#pragma once

#include "Types/Array.h"

#include <vulkan/vulkan.h>

namespace Quartz
{
	struct VulkanSurface
	{
		VkSurfaceKHR				vkSurface;
		Array<VkSurfaceFormatKHR>	supportedFormats;
		VkSurfaceCapabilitiesKHR	capibilites;

		uInt32						width;
		uInt32						height;
	};
}