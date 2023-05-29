#pragma once

#include "Types/Types.h"

#include <vulkan/vulkan.h>

namespace Quartz
{
	class VulkanImage
	{
	private:
		VkImage				mvkImage;
		VkDeviceMemory		mvkMemory;
		VkImageType			mvkImageType;
		VkFormat			mvkFormat;
		VkImageUsageFlags	mvkUsageFlags;

		uInt32				mWidth;
		uInt32				mHeight;
		uInt32				mDepth;
		uInt32				mLayers;
		uInt32				mMips;

	public:
		VulkanImage() {};
		VulkanImage(VkImage vkImage, VkDeviceMemory vkMemory, VkFormat vkFormat,
			VkImageUsageFlags vkUsage, VkImageType vkImageType,
			uInt32 width, uInt32 height, uInt32 depth, uInt32 layers, uInt32 mips);

		VkImage				GetVkImage() { return mvkImage; }
		VkImageType			GetVkImageType() { return mvkImageType; }
		VkFormat			GetVkFormat() { return mvkFormat; }
		VkImageUsageFlags	GetVkImageUsageFlags() { return mvkUsageFlags; }
		VkDeviceMemory		GetVkDeviceMemory() { return mvkMemory; }
		uInt32				GetWidth() const { return mWidth; }
		uInt32				GetHeight() const { return mHeight; }
		uInt32				GetDepth() const { return mDepth; }
		uInt32				GetLayers() const { return mLayers; }
		uInt32				GetMips() const { return mMips; }
	};

	class VulkanImageView
	{
	private:
		VkImageView			mvkImageView;
		VkImageViewType		mvkViewType;
		VkImageAspectFlags	mvkAspectFlags;

		VulkanImage*		mpImage;

		uInt32				mLayerStart;
		uInt32				mLayerCount;
		uInt32				mMipStart;
		uInt32				mMipCount;

	public:
		VulkanImageView() {};
		VulkanImageView(VkImageView vkImageView, VkImageViewType vkViewType,
			VkImageAspectFlags vkAspectFlags, VulkanImage* pImage,
			uInt32 layerStart, uInt32 layerCount, uInt32 mipStart, uInt32 mipCount);

		VkImageView			GetVkImageView() { return mvkImageView; }
		VkImageViewType		GetVkImageViewType() { return mvkViewType; }
		VkImageAspectFlags	GetVkImageAspectFlags() { return mvkAspectFlags; }
		VulkanImage*		GetImage() { return mpImage; }
		uInt32				GetLayerStart() const { return mLayerStart; }
		uInt32				GetLayerCount() const { return mLayerCount; }
		uInt32				GetMipStart() const { return mMipStart; }
		uInt32				GetMipCount() const { return mMipCount; }
	};
}