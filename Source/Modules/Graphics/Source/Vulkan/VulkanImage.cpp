#include "Vulkan/VulkanImage.h"

namespace Quartz
{
	VulkanImage::VulkanImage(
		VkImage vkImage, VkDeviceMemory vkMemory, VkFormat vkFormat, VkImageUsageFlags vkUsage, VkImageType vkImageType,
		uInt32 width, uInt32 height, uInt32 depth, uInt32 layers, uInt32 mips) :
		mvkImage(vkImage),
		mvkMemory(vkMemory),
		mvkFormat(vkFormat),
		mvkUsageFlags(vkUsage),
		mvkImageType(vkImageType),
		mWidth(width),
		mHeight(height),
		mDepth(depth),
		mLayers(layers),
		mMips(mips)
	{ }

	VulkanImageView::VulkanImageView(
		VkImageView vkImageView, VkImageViewType vkViewType, VkImageAspectFlags vkAspectFlags, VulkanImage* pImage,
		uInt32 layerStart, uInt32 layerCount, uInt32 mipStart, uInt32 mipCount) :
		mvkImageView(vkImageView),
		mvkViewType(vkViewType),
		mvkAspectFlags(vkAspectFlags),
		mpImage(pImage),
		mLayerStart(layerStart),
		mLayerCount(layerCount),
		mMipStart(mipStart),
		mMipCount(mipCount)
	{ }
}

