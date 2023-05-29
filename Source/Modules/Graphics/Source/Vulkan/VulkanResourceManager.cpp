#include "Vulkan/VulkanResourceManager.h"

#include "Log.h"

#include "Vulkan/VulkanDevice.h"
#include "Vulkan/VulkanPhysicalDevice.h"

#include "Vulkan/VulkanApiSurface.h"

#include <vulkan/vulkan.h>

namespace Quartz
{
	VulkanImage* VulkanResourceManager::RegisterImage(const VulkanImage& image)
	{
		return &mImages.PushBack(image);
	}

	VulkanImageView* VulkanResourceManager::RegisterImageView(const VulkanImageView& imageView)
	{
		return &mImageViews.PushBack(imageView);
	}

	bool EnumeratePresentModes(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, Array<VkPresentModeKHR>& presentModes)
	{
		VkResult result;
		uInt32 modeCount = 0;

		result = vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &modeCount, nullptr);
		presentModes.Resize(modeCount);
		result = vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &modeCount, presentModes.Data());

		return result == VK_SUCCESS;
	}

	VkResult EnumerateSurfaceFormats(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, Array<VkSurfaceFormatKHR>& surfaceFormats)
	{
		VkResult result;
		uInt32 formatCount = 0;

		result = vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, nullptr);
		surfaceFormats.Resize(formatCount);
		result = vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, surfaceFormats.Data());

		return result;
	}

	VulkanSurface* VulkanResourceManager::CreateSurface(VkInstance vkInstance, VulkanDevice* pDevice, VulkanApiSurface* pSurface)
	{
		VkSurfaceKHR vkSurface = pSurface->GetVkSurface();
		VkPhysicalDevice vkPhysicalDevice = pDevice->pPhysicalDevice->vkPhysicalDevice;

		Array<VkSurfaceFormatKHR>	supportedFormats;
		VkSurfaceCapabilitiesKHR	surfaceCapibilites;

		if (EnumerateSurfaceFormats(vkPhysicalDevice, vkSurface, supportedFormats) != VK_SUCCESS)
		{
			LogError("Failed to enumerate win32 vulkan surface formats!");
			vkDestroySurfaceKHR(vkInstance, vkSurface, VK_NULL_HANDLE);
			return nullptr;
		}

		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(vkPhysicalDevice, vkSurface, &surfaceCapibilites);

		uInt32 width = surfaceCapibilites.maxImageExtent.width;
		uInt32 height = surfaceCapibilites.maxImageExtent.height;

		VulkanSurface& vulkanSurface = mSurfaces.PushBack(
			VulkanSurface(vkSurface, width, height, supportedFormats, surfaceCapibilites)
		);

		return &vulkanSurface;
	}

	bool PickSurfaceFormat(VkPhysicalDevice physicalDevice, VulkanSurface* pSurface, bool preferHDR, VkSurfaceFormatKHR* pSelectedFormat)
	{
		Array<VkSurfaceFormatKHR> availableFormats;

		for (const VkSurfaceFormatKHR& format : pSurface->GetSupportedFormats())
		{
			if (preferHDR)
			{
				if (format.format == VK_FORMAT_A2B10G10R10_SNORM_PACK32 && format.colorSpace == VK_COLOR_SPACE_HDR10_ST2084_EXT)
				{
					*pSelectedFormat = format;
					return true;
				}
			}
			else
			{
				if (format.format == VK_FORMAT_B8G8R8A8_UNORM && format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
				{
					*pSelectedFormat = format;
					return true;
				}
			}
		}

		// No prefered format found, use default
		*pSelectedFormat = availableFormats[0];

		return true;
	}

	bool PickPresentationMode(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, bool vSync, VkPresentModeKHR* pSelectedPresentMode)
	{
		Array<VkPresentModeKHR> availablePresentModes;

		if (!EnumeratePresentModes(physicalDevice, surface, availablePresentModes))
		{
			LogWarning("Failed to pick surface format: Unable to enumerate present modes!");
			return false;
		}

		if (!vSync && availablePresentModes.Contains(VK_PRESENT_MODE_IMMEDIATE_KHR))
		{
			*pSelectedPresentMode = VK_PRESENT_MODE_IMMEDIATE_KHR;
			return true;
		}

		else if (availablePresentModes.Contains(VK_PRESENT_MODE_MAILBOX_KHR))
		{
			*pSelectedPresentMode = VK_PRESENT_MODE_MAILBOX_KHR;
			return true;
		}

		// VK_PRESENT_MODE_FIFO_KHR is guaranteed by the spec
		*pSelectedPresentMode = VK_PRESENT_MODE_FIFO_KHR;
		return true;
	}

	VkResult CreateVkImageView(VkDevice vkDevice, VkImageView* pVkImageView, VkImage vkImage,
		VkImageViewType vkImageViewType, VkImageAspectFlags vkAspectFlags, VkFormat vkFormat,
		uInt32 layerStart, uInt32 layerCount, uInt32 mipStart, uInt32 mipCount)
	{
		VkImageView vkImageView;

		VkImageViewCreateInfo vkImageViewInfo = {};
		vkImageViewInfo.sType		= VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		vkImageViewInfo.flags		= 0;
		vkImageViewInfo.image		= vkImage;
		vkImageViewInfo.viewType	= vkImageViewType;
		vkImageViewInfo.format		= vkFormat;

		vkImageViewInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		vkImageViewInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		vkImageViewInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		vkImageViewInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

		vkImageViewInfo.subresourceRange.aspectMask			= vkAspectFlags;
		vkImageViewInfo.subresourceRange.baseMipLevel		= mipStart;
		vkImageViewInfo.subresourceRange.levelCount			= mipCount;
		vkImageViewInfo.subresourceRange.baseArrayLayer		= layerStart;
		vkImageViewInfo.subresourceRange.layerCount			= layerCount;

		VkResult result = vkCreateImageView(vkDevice, &vkImageViewInfo, nullptr, &vkImageView);

		if (result != VK_SUCCESS)
		{
			LogError("Failed to create vulkan image view: vkCreateImageView failed!");
			return result;
		}

		*pVkImageView = vkImageView;

		return result;
	}

	VulkanImageView* VulkanResourceManager::CreateImageView(VulkanDevice* pDevice, VulkanImage* pImage,
		VkImageViewType vkImageViewType, VkImageAspectFlags vkAspectFlags, VkFormat vkFormat, 
		uInt32 mipStart, uInt32 mipCount, uInt32 layerStart, uInt32 layerCount)
	{
		VkImageView vkImageView;

		VkResult result = CreateVkImageView(pDevice->vkDevice, &vkImageView, pImage->GetVkImage(), 
			vkImageViewType, vkAspectFlags, vkFormat, mipStart, mipCount, layerStart, layerCount);

		if (result != VK_SUCCESS)
		{
			return nullptr;
		}

		VulkanImageView* pImageView = RegisterImageView(
			VulkanImageView(vkImageView, vkImageViewType, vkAspectFlags, pImage, layerStart, layerCount, mipStart, mipCount)
		);

		return pImageView;
	}

	VulkanSwapchain* VulkanResourceManager::CreateSwapchain(VulkanDevice* pDevice, VulkanSurface* pSurface, uInt32 bufferCount)
	{
		VkSwapchainKHR		vkSwapchain;
		VkSurfaceFormatKHR	selectedFormat;
		VkPresentModeKHR	selectedPresentMode;

		VkResult result;

		VkPhysicalDevice physicalDevice = pDevice->pPhysicalDevice->vkPhysicalDevice;
		VkSurfaceKHR vkSurface = pSurface->GetVkSurface();

		if (!PickSurfaceFormat(physicalDevice, pSurface, false, &selectedFormat))
		{
			LogError("Failed to create vulkan swapchain: No suitable surface format was detected!");
			return nullptr;
		}

		if (!PickPresentationMode(physicalDevice, vkSurface, false, &selectedPresentMode))
		{
			LogError("Failed to create vulkan swapchain: No suitable present mode was detected!");
			return nullptr;
		}

		VkSurfaceCapabilitiesKHR surfaceCapabilites = pSurface->GetCapibilites();

		VkSurfaceTransformFlagBitsKHR preTransform = surfaceCapabilites.currentTransform;
		if (surfaceCapabilites.supportedTransforms & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR)
		{
			preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
		}

		VkCompositeAlphaFlagBitsKHR compositeAlpha = VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR;
		if (surfaceCapabilites.supportedCompositeAlpha & VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR)
		{
			compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		}

		VkBool32 supportsPresent = false;
		if (vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, 
			pDevice->pPhysicalDevice->primaryQueueFamilyIndices.present, vkSurface, &supportsPresent) != VK_SUCCESS)
		{
			LogError("Failed to create vulkan swapchain: Specified device and queue do not support presentation!");
			return nullptr;
		}

		uInt32 imageCount = bufferCount;

		if (bufferCount < surfaceCapabilites.minImageCount)
		{
			imageCount = surfaceCapabilites.minImageCount;
			LogWarning("Swapchain only supports %d backbuffers. %d requested.", imageCount, bufferCount);
		}

		VkSwapchainCreateInfoKHR swapChainInfo = {};
		swapChainInfo.sType				= VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		swapChainInfo.surface			= vkSurface;
		swapChainInfo.minImageCount		= imageCount;
		swapChainInfo.imageFormat		= selectedFormat.format;
		swapChainInfo.imageColorSpace	= selectedFormat.colorSpace;
		swapChainInfo.imageExtent		= surfaceCapabilites.currentExtent;
		swapChainInfo.imageArrayLayers	= 1;
		swapChainInfo.imageUsage		= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
		swapChainInfo.imageSharingMode	= VK_SHARING_MODE_EXCLUSIVE;
		swapChainInfo.preTransform		= preTransform;
		swapChainInfo.compositeAlpha	= compositeAlpha;
		swapChainInfo.presentMode		= selectedPresentMode;
		swapChainInfo.clipped			= VK_TRUE;
		swapChainInfo.oldSwapchain		= VK_NULL_HANDLE;
		swapChainInfo.pNext				= nullptr;

		if (vkCreateSwapchainKHR(pDevice->vkDevice, &swapChainInfo, nullptr, &vkSwapchain) != VK_SUCCESS)
		{
			LogError("Failed to create vulkan swapchain: vkCreateSwapchainKHR failed!");
			return nullptr;
		}

		uInt32			swapChainImageCount = 0;
		Array<VkImage>	swapchainImages;

		result = vkGetSwapchainImagesKHR(pDevice->vkDevice, vkSwapchain, &swapChainImageCount, nullptr);
		swapchainImages.Resize(swapChainImageCount);
		result = vkGetSwapchainImagesKHR(pDevice->vkDevice, vkSwapchain, &swapChainImageCount, swapchainImages.Data());

		if (result != VK_SUCCESS)
		{
			LogError("Failed to retrieve images from swapchain!");
			vkDestroySwapchainKHR(pDevice->vkDevice, vkSwapchain, VK_NULL_HANDLE);
			return nullptr;
		}

		Array<VulkanImage*>		images;
		Array<VulkanImageView*>	imageViews;
		Array<VkSemaphore>		imageAvailableSemaphores;
		Array<VkSemaphore>		imageCompleteSemaphores;
		Array<VkFence>			imageFences;
		Array<VkFence>			inFlightFences;

		images.Resize(swapChainImageCount);
		imageViews.Resize(swapChainImageCount);
		imageAvailableSemaphores.Resize(swapChainImageCount);
		imageCompleteSemaphores.Resize(swapChainImageCount);
		imageFences.Resize(swapChainImageCount);
		inFlightFences.Resize(swapChainImageCount, VK_NULL_HANDLE);

		for (uInt32 i = 0; i < swapChainImageCount; i++)
		{
			VkImage		vkImage = swapchainImages[i];
			VkImageView vkImageView;

			VulkanImage* pImage = RegisterImage(VulkanImage
			(
				vkImage,
				VK_NULL_HANDLE,
				selectedFormat.format,
				VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
				VK_IMAGE_TYPE_2D,
				swapChainInfo.imageExtent.width,
				swapChainInfo.imageExtent.height,
				1,
				1, 1
			));

			VkResult result = CreateVkImageView
			(
				pDevice->vkDevice,
				&vkImageView,
				vkImage,
				VK_IMAGE_VIEW_TYPE_2D,
				VK_IMAGE_ASPECT_COLOR_BIT,
				selectedFormat.format,
				0, 1, 0, 1
			);

			if (result != VK_SUCCESS)
			{
				LogError("Failed to create swapchain: unable to create image views.");
				vkDestroySwapchainKHR(pDevice->vkDevice, vkSwapchain, VK_NULL_HANDLE);
				return nullptr;
			}

			VulkanImageView* pImageView = RegisterImageView(VulkanImageView
			(
				vkImageView,
				VK_IMAGE_VIEW_TYPE_2D,
				VK_IMAGE_ASPECT_COLOR_BIT,
				pImage,
				1, 0,
				1, 0
			));

			VkSemaphoreTypeCreateInfo vkSemaphoreType = {};
			vkSemaphoreType.sType			= VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO;
			vkSemaphoreType.semaphoreType	= VK_SEMAPHORE_TYPE_BINARY;
			vkSemaphoreType.initialValue	= 0;

			VkSemaphoreCreateInfo semaphoreInfo = {};
			semaphoreInfo.sType				= VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
			semaphoreInfo.pNext				= &vkSemaphoreType;
			semaphoreInfo.flags				= 0;

			VkFenceCreateInfo fenceInfo = {};
			fenceInfo.sType					= VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
			fenceInfo.pNext					= nullptr;
			fenceInfo.flags					= VK_FENCE_CREATE_SIGNALED_BIT; // Start signaled

			vkCreateSemaphore(pDevice->vkDevice, &semaphoreInfo, nullptr, &imageAvailableSemaphores[i]);
			vkCreateSemaphore(pDevice->vkDevice, &semaphoreInfo, nullptr, &imageCompleteSemaphores[i]);
			vkCreateFence(pDevice->vkDevice, &fenceInfo, nullptr, &inFlightFences[i]);

			//TransitionImage(pImage, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, 1);

			images[i]		= pImage;
			imageViews[i]	= pImageView;
		}

		VulkanSwapchain* pVulkanSwapchain = new VulkanSwapchain
		(
			vkSwapchain, pDevice, imageCount, images, imageViews, imageAvailableSemaphores,
			imageCompleteSemaphores, imageFences, inFlightFences
		);

		return pVulkanSwapchain;
	}
}