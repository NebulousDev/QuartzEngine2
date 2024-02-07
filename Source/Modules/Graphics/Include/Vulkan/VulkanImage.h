#pragma once

#include "Types/Types.h"

#include <vulkan/vulkan.h>

namespace Quartz
{
	struct VulkanImage
	{
		VkImage				vkImage;
		VkDeviceMemory		vkMemory;
		VkImageType			vkImageType;
		VkFormat			vkFormat;
		VkImageUsageFlags	vkUsageFlags;

		uInt32				width;
		uInt32				height;
		uInt32				depth;
		uInt32				layers;
		uInt32				mips;
	};

	struct VulkanImageView
	{
		VkImageView			vkImageView;
		VkImageViewType		vkImageViewType;
		VkImageAspectFlags	vkAspectFlags;

		VulkanImage*		pImage;

		uInt32				layerStart;
		uInt32				layerCount;
		uInt32				mipStart;
		uInt32				mipCount;
	};
}