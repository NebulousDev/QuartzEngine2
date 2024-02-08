#pragma once

#include "Types/Types.h"

#include <vulkan/vulkan.h>

namespace Quartz
{
	struct VulkanDevice;

	struct VulkanImageInfo
	{
		VkImageType			vkImageType;
		VkFormat			vkFormat;
		VkImageUsageFlags	vkUsageFlags;
		uInt32				width;
		uInt32				height;
		uInt32				depth;
		uInt32				layers;
		uInt32				mips;
	};

	struct VulkanImage
	{
		VkImage				vkImage;
		VulkanDevice*		pDevice;
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

	struct VulkanImageViewInfo
	{
		VulkanImage*		pImage;
		VkImageViewType		vkImageViewType;
		VkImageAspectFlags	vkAspectFlags;
		VkFormat			vkFormat;
		uInt32				mipStart;
		uInt32				mipCount;
		uInt32				layerStart;
		uInt32				layerCount;
	};

	struct VulkanImageView
	{
		VkImageView			vkImageView;
		VulkanDevice*		pDevice;
		VulkanImage*		pImage;
		VkImageViewType		vkImageViewType;
		VkImageAspectFlags	vkAspectFlags;
		uInt32				layerStart;
		uInt32				layerCount;
		uInt32				mipStart;
		uInt32				mipCount;
	};
}