#pragma once

#include "../GfxDLL.h"
#include "Types/Array.h"

#include "Vulkan/VulkanSurface.h"
#include "Vulkan/VulkanSwapchain.h"
#include "Vulkan/VulkanImage.h"

#include <vulkan/vulkan.h>

namespace Quartz
{
	class VulkanDevice;
	class VulkanSurface;
	class VulkanSwapchain; 

	class VulkanApiSurface;

	class QUARTZ_GRAPHICS_API VulkanResourceManager
	{
	private:
		Array<VulkanSurface>	mSurfaces;
		Array<VulkanSwapchain>	mSwapchains;
		Array<VulkanImage>		mImages;
		Array<VulkanImageView>	mImageViews;

		VulkanImage*			RegisterImage(const VulkanImage& image);
		VulkanImageView*		RegisterImageView(const VulkanImageView& imageView);

	public:
		VulkanSurface*		CreateSurface(VkInstance vkInstance, VulkanDevice* pDevice, VulkanApiSurface* pSurface);
		VulkanSwapchain*	CreateSwapchain(VulkanDevice* pDevice, VulkanSurface* pSurface, uInt32 bufferCount);
		VulkanImageView*	CreateImageView(VulkanDevice* pDevice, VulkanImage* pImage,
								VkImageViewType vkImageViewType, VkImageAspectFlags vkAspectFlags, VkFormat vkFormat,
								uInt32 mipStart, uInt32 mipLevels, uInt32 layerStart, uInt32 layers);
	};
}