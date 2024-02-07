#include "Vulkan/VulkanResourceManager.h"

#include "Log.h"

#include "Vulkan/VulkanDevice.h"
#include "Vulkan/VulkanPhysicalDevice.h"

#include "Vulkan/VulkanApiSurface.h"

#include <vulkan/vulkan.h>

namespace Quartz
{
	VulkanSurface* VulkanResourceManager::Register(const VulkanSurface& surface)
	{
		return &mSurfaces.PushBack(surface);
	}

	VulkanSwapchain* VulkanResourceManager::Register(const VulkanSwapchain& swapchain)
	{
		return &mSwapchains.PushBack(swapchain);
	}

	VulkanImage* VulkanResourceManager::Register(const VulkanImage& image)
	{
		return &mImages.PushBack(image);
	}

	VulkanImageView* VulkanResourceManager::Register(const VulkanImageView& imageView)
	{
		return &mImageViews.PushBack(imageView);
	}

	VulkanShader* VulkanResourceManager::Register(const VulkanShader& shader)
	{
		return &mShaders.PushBack(shader);
	}

	VulkanRenderpass* VulkanResourceManager::Register(const VulkanRenderpass& renderpass)
	{
		return &mRenderpasss.PushBack(renderpass);
	}

	VulkanGraphicsPipeline* VulkanResourceManager::Register(const VulkanGraphicsPipeline& pipeline)
	{
		return &mGraphicsPipelines.PushBack(pipeline);
	}

	VulkanBuffer* VulkanResourceManager::Register(const VulkanBuffer& buffer)
	{
		return &mBuffers.PushBack(buffer);
	}

	VulkanCommandPool* VulkanResourceManager::Register(const VulkanCommandPool& pool)
	{
		return &mCommandPools.PushBack(pool);
	}

	VulkanCommandBuffer* VulkanResourceManager::Register(const VulkanCommandBuffer& buffer)
	{
		return &mCommandBuffers.PushBack(buffer);
	}

	VulkanFramebuffer* VulkanResourceManager::Register(const VulkanFramebuffer& framebuffer)
	{
		return &mFramebuffers.PushBack(framebuffer);
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

	VulkanSurface* VulkanResourceManager::CreateSurface(VkInstance vkInstance, const VulkanDevice& device, const VulkanApiSurface& surface)
	{
		VkSurfaceKHR vkSurface = surface.GetVkSurface();
		VkPhysicalDevice vkPhysicalDevice = device.pPhysicalDevice->vkPhysicalDevice;

		Array<VkSurfaceFormatKHR>	supportedFormats;
		VkSurfaceCapabilitiesKHR	surfaceCapibilites;

		if (EnumerateSurfaceFormats(vkPhysicalDevice, vkSurface, supportedFormats) != VK_SUCCESS)
		{
			LogError("Failed to enumerate win32 vulkan surface formats!");
			vkDestroySurfaceKHR(vkInstance, vkSurface, VK_NULL_HANDLE);
			return nullptr;
		}

		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(vkPhysicalDevice, vkSurface, &surfaceCapibilites);

		LogTrace("Created VulkanSerface [ID=%06.6d].", mSurfaces.Size() + 1);

		VulkanSurface vulkanSurface		= {};
		vulkanSurface.vkSurface			= vkSurface;
		vulkanSurface.supportedFormats	= supportedFormats;
		vulkanSurface.capibilites		= surfaceCapibilites;
		vulkanSurface.width				= surfaceCapibilites.maxImageExtent.width;
		vulkanSurface.height			= surfaceCapibilites.maxImageExtent.height;

		return Register(vulkanSurface);
	}

	bool PickSurfaceFormat(VkPhysicalDevice physicalDevice, const VulkanSurface& surface, bool preferHDR, VkSurfaceFormatKHR* pSelectedFormat)
	{
		Array<VkSurfaceFormatKHR> availableFormats;

		for (const VkSurfaceFormatKHR& format : surface.supportedFormats)
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

	VulkanImageView* VulkanResourceManager::CreateImageView(const VulkanDevice& device, VulkanImage& image,
		VkImageViewType vkImageViewType, VkImageAspectFlags vkAspectFlags, VkFormat vkFormat,
		uInt32 mipStart, uInt32 mipCount, uInt32 layerStart, uInt32 layerCount)
	{
		VkImageView vkImageView;

		VkResult result = CreateVkImageView(device.vkDevice, &vkImageView, image.vkImage,
			vkImageViewType, vkAspectFlags, vkFormat, mipStart, mipCount, layerStart, layerCount);

		if (result != VK_SUCCESS)
		{
			return nullptr;
		}

		LogTrace("Created VulkanImageView [ID=%06.6d].", mSwapchains.Size() + 1);

		VulkanImageView vulkanImageView		= {};
		vulkanImageView.vkImageViewType		= vkImageViewType;
		vulkanImageView.vkAspectFlags		= vkAspectFlags;
		vulkanImageView.pImage				= &image;
		vulkanImageView.layerStart			= layerStart;
		vulkanImageView.layerCount			= layerCount;
		vulkanImageView.mipStart			= mipStart;
		vulkanImageView.mipCount			= mipCount;

		return Register(vulkanImageView);
	}

	VulkanSwapchain* VulkanResourceManager::CreateSwapchain(VulkanDevice& device, const VulkanSurface& surface, uInt32 bufferCount)
	{
		VkSwapchainKHR		vkSwapchain;
		VkSurfaceFormatKHR	selectedFormat;
		VkPresentModeKHR	selectedPresentMode;

		VkResult result;

		VkPhysicalDevice physicalDevice = device.pPhysicalDevice->vkPhysicalDevice;
		VkSurfaceKHR vkSurface = surface.vkSurface;

		if (!PickSurfaceFormat(physicalDevice, surface, false, &selectedFormat))
		{
			LogError("Failed to create vulkan swapchain: No suitable surface format was detected!");
			return nullptr;
		}

		if (!PickPresentationMode(physicalDevice, vkSurface, false, &selectedPresentMode))
		{
			LogError("Failed to create vulkan swapchain: No suitable present mode was detected!");
			return nullptr;
		}

		VkSurfaceCapabilitiesKHR surfaceCapabilites = surface.capibilites;

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
			device.pPhysicalDevice->primaryQueueFamilyIndices.present, vkSurface, &supportsPresent) != VK_SUCCESS)
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

		if (vkCreateSwapchainKHR(device.vkDevice, &swapChainInfo, nullptr, &vkSwapchain) != VK_SUCCESS)
		{
			LogError("Failed to create vulkan swapchain: vkCreateSwapchainKHR failed!");
			return nullptr;
		}

		uInt32			swapChainImageCount = 0;
		Array<VkImage>	swapchainImages;

		result = vkGetSwapchainImagesKHR(device.vkDevice, vkSwapchain, &swapChainImageCount, nullptr);
		swapchainImages.Resize(swapChainImageCount);
		result = vkGetSwapchainImagesKHR(device.vkDevice, vkSwapchain, &swapChainImageCount, swapchainImages.Data());

		if (result != VK_SUCCESS)
		{
			LogError("Failed to retrieve images from swapchain!");
			vkDestroySwapchainKHR(device.vkDevice, vkSwapchain, VK_NULL_HANDLE);
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

		for (uInt32 i = 0; i < swapChainImageCount; i++)
		{
			VulkanImage vulkanImage		= {};
			vulkanImage.vkImage			= swapchainImages[i];
			vulkanImage.vkMemory		= VK_NULL_HANDLE;
			vulkanImage.vkFormat		= selectedFormat.format;
			vulkanImage.vkUsageFlags	= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
			vulkanImage.width			= swapChainInfo.imageExtent.width;
			vulkanImage.height			= swapChainInfo.imageExtent.height;
			vulkanImage.depth			= 1;
			vulkanImage.layers			= 1;
			vulkanImage.mips			= 1;

			VulkanImage* pImage = Register(vulkanImage);

			VkImageView vkImageView;

			VkResult result = CreateVkImageView
			(
				device.vkDevice,
				&vkImageView,
				vulkanImage.vkImage,
				VK_IMAGE_VIEW_TYPE_2D,
				VK_IMAGE_ASPECT_COLOR_BIT,
				selectedFormat.format,
				0, 1, 0, 1
			);

			if (result != VK_SUCCESS)
			{
				LogError("Failed to create swapchain: unable to create image views.");
				vkDestroySwapchainKHR(device.vkDevice, vkSwapchain, VK_NULL_HANDLE);
				return nullptr;
			}

			VulkanImageView vulkanImageView		= {};
			vulkanImageView.vkImageView			= vkImageView;
			vulkanImageView.vkImageViewType		= VK_IMAGE_VIEW_TYPE_2D;
			vulkanImageView.vkAspectFlags		= VK_IMAGE_ASPECT_COLOR_BIT;
			vulkanImageView.pImage				= pImage;
			vulkanImageView.layerStart			= 0;
			vulkanImageView.layerCount			= 1;
			vulkanImageView.mipStart			= 0;
			vulkanImageView.mipCount			= 1;

			VulkanImageView* pImageView = Register(vulkanImageView);

			VkSemaphoreTypeCreateInfo vkSemaphoreType = {};
			vkSemaphoreType.sType			= VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO;
			vkSemaphoreType.semaphoreType	= VK_SEMAPHORE_TYPE_BINARY;
			vkSemaphoreType.initialValue	= 0;

			VkSemaphoreCreateInfo semaphoreInfo = {};
			semaphoreInfo.sType				= VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
			semaphoreInfo.flags				= 0;
			semaphoreInfo.pNext				= &vkSemaphoreType;

			VkFenceCreateInfo fenceInfo = {};
			fenceInfo.sType					= VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
			fenceInfo.flags					= VK_FENCE_CREATE_SIGNALED_BIT; // Start signaled
			fenceInfo.pNext					= nullptr;

			vkCreateSemaphore(device.vkDevice, &semaphoreInfo, nullptr, &imageAvailableSemaphores[i]);
			vkCreateSemaphore(device.vkDevice, &semaphoreInfo, nullptr, &imageCompleteSemaphores[i]);
			vkCreateFence(device.vkDevice, &fenceInfo, nullptr, &imageFences[i]);

			//TransitionImage(pImage, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, 1);

			images[i]		= pImage;
			imageViews[i]	= pImageView;
		}

		LogTrace("Created VulkanSwapchain [ID=%06.6d].", mSwapchains.Size() + 1);

		VulkanSwapchain vulkanSwapchain = {};
		vulkanSwapchain.vkSwapchain					= vkSwapchain;
		vulkanSwapchain.backbufferCount				= bufferCount;
		vulkanSwapchain.pDevice						= &device;
		vulkanSwapchain.images						= images;
		vulkanSwapchain.imageViews					= imageViews;
		vulkanSwapchain.imageAvailableSemaphores	= imageAvailableSemaphores;
		vulkanSwapchain.imageCompleteSemaphores		= imageCompleteSemaphores;
		vulkanSwapchain.imageFences					= imageFences;

		return Register(vulkanSwapchain);
	}

	VulkanShader* VulkanResourceManager::CreateShader(const VulkanDevice& device, const String& name, const Array<uInt8>& binary)
	{
		VkShaderModule			vkShader;
		SpirvReflection			reflection;
		Array<SpirvUniform>		uniforms;
		Array<SpirvAttribute>	attributes;
		VkShaderStageFlagBits	vkStageFlags = {};

		VkShaderModuleCreateInfo shaderInfo = {};
		shaderInfo.sType	= VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		shaderInfo.codeSize = binary.Size();
		shaderInfo.pCode	= reinterpret_cast<const uint32_t*>(binary.Data());

		if (vkCreateShaderModule(device.vkDevice, &shaderInfo, nullptr, &vkShader) != VK_SUCCESS)
		{
			LogError("Failed to create shader: vkCreateShaderModule failed!");
			return nullptr;
		}

		if (!SpirvParseReflection(&reflection, binary))
		{
			LogError("Failed to create shader: Failed to parse SPIR-V reflection data!");
			vkDestroyShaderModule(device.vkDevice, vkShader, VK_NULL_HANDLE);
			return nullptr;
		}

		SpirvExtractUniforms(uniforms, reflection);
		SpirvExtractAttributes(attributes, reflection);

		switch (reflection.executionModel)
		{
			case SpvExecutionModelVertex:
			{
				vkStageFlags	= VK_SHADER_STAGE_VERTEX_BIT;

				break;
			}
			case SpvExecutionModelTessellationControl:
			{
				vkStageFlags	= VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;

				break;
			}
			case SpvExecutionModelTessellationEvaluation:
			{
				vkStageFlags	= VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;

				break;
			}
			case SpvExecutionModelGeometry:
			{
				vkStageFlags	= VK_SHADER_STAGE_GEOMETRY_BIT;

				break;
			}
			case SpvExecutionModelFragment:
			{
				vkStageFlags	= VK_SHADER_STAGE_FRAGMENT_BIT;

				break;
			}
			case SpvExecutionModelGLCompute:
			{
				vkStageFlags	= VK_SHADER_STAGE_COMPUTE_BIT;

				break;
			}
			case SpvExecutionModelKernel:
			{
				vkStageFlags	= VK_SHADER_STAGE_COMPUTE_BIT; // Correct?

				break;
			}
			case SpvExecutionModelTaskNV:
			{
				vkStageFlags	= VK_SHADER_STAGE_TASK_BIT_NV;

				break;
			}
			case SpvExecutionModelMeshNV:
			{
				vkStageFlags	= VK_SHADER_STAGE_MESH_BIT_NV;

				break;
			}
			case SpvExecutionModelRayGenerationKHR:
			{
				vkStageFlags	= VK_SHADER_STAGE_RAYGEN_BIT_KHR;

				break;
			}
			case SpvExecutionModelIntersectionKHR:
			{
				vkStageFlags	= VK_SHADER_STAGE_INTERSECTION_BIT_KHR;

				break;
			}
			case SpvExecutionModelAnyHitKHR:
			{
				vkStageFlags	= VK_SHADER_STAGE_ANY_HIT_BIT_KHR;

				break;
			}
			case SpvExecutionModelClosestHitKHR:
			{
				vkStageFlags	= VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR;

				break;
			}
			case SpvExecutionModelMissKHR:
			{
				vkStageFlags	= VK_SHADER_STAGE_MISS_BIT_KHR;

				break;
			}
			case SpvExecutionModelCallableKHR:
			{
				vkStageFlags	= VK_SHADER_STAGE_CALLABLE_BIT_KHR;

				break;
			}
		}

		LogTrace("Created VulkanShader [ID=%06.6d].", mShaders.Size() + 1);

		VulkanShader vulkanShader = {};
		vulkanShader.name		= name;
		vulkanShader.vkShader	= vkShader;
		vulkanShader.vkStage	= vkStageFlags;
		vulkanShader.entryPoint	= reflection.entryName;
		vulkanShader.uniforms	= uniforms;
		vulkanShader.attributes	= attributes;

		return Register(vulkanShader);
	}

	VulkanRenderpass* VulkanResourceManager::CreateRenderpass(const VulkanDevice& device, const VulkanRenderpassInfo& info)
	{
		VkRenderPass vkRenderPass;

		Array<VkAttachmentDescription>		attachmentDescriptions;
		Array<VkSubpassDescription>			subpassDescriptions;
		Array<Array<VkAttachmentReference>> subpassColorAttachmentReferences;
		Array<Array<VkAttachmentReference>> subpassInputAttachmentReferences;
		Array<VkAttachmentReference>		subpassDepthAttachmentReferences;
		Array<VkSubpassDependency>			subpassDependancies;

		subpassColorAttachmentReferences.Resize(info.subpasses.Size());
		subpassInputAttachmentReferences.Resize(info.subpasses.Size());
		subpassDepthAttachmentReferences.Resize(info.subpasses.Size());

		bool hasColor	= false;
		bool hasDepth	= false;
		bool hasStencil	= false;

		for (const VulkanAttachment& attachment : info.attachments)
		{
			VkAttachmentDescription vkAttachmentDescription = {};
			vkAttachmentDescription.flags	= 0;
			vkAttachmentDescription.format	= attachment.vkFormat;
			vkAttachmentDescription.samples = VK_SAMPLE_COUNT_1_BIT;

			if (attachment.type == VULKAN_ATTACHMENT_TYPE_SWAPCHAIN)
			{
				vkAttachmentDescription.loadOp			= VK_ATTACHMENT_LOAD_OP_CLEAR;
				vkAttachmentDescription.storeOp			= VK_ATTACHMENT_STORE_OP_STORE;
				vkAttachmentDescription.stencilLoadOp	= VK_ATTACHMENT_LOAD_OP_DONT_CARE;
				vkAttachmentDescription.stencilStoreOp	= VK_ATTACHMENT_STORE_OP_DONT_CARE;
				vkAttachmentDescription.initialLayout	= VK_IMAGE_LAYOUT_UNDEFINED;
				vkAttachmentDescription.finalLayout		= VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

				hasColor = true;
			}
			else if (attachment.type == VULKAN_ATTACHMENT_TYPE_DEPTH)
			{
				vkAttachmentDescription.loadOp			= VK_ATTACHMENT_LOAD_OP_CLEAR;
				vkAttachmentDescription.storeOp			= VK_ATTACHMENT_STORE_OP_STORE;
				vkAttachmentDescription.stencilLoadOp	= VK_ATTACHMENT_LOAD_OP_DONT_CARE;
				vkAttachmentDescription.stencilStoreOp	= VK_ATTACHMENT_STORE_OP_DONT_CARE;
				vkAttachmentDescription.initialLayout	= VK_IMAGE_LAYOUT_UNDEFINED;
				vkAttachmentDescription.finalLayout		= VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;

				hasDepth = true;
			}
			else if (attachment.type == VULKAN_ATTACHMENT_TYPE_DEPTH_STENCIL)
			{
				vkAttachmentDescription.loadOp			= VK_ATTACHMENT_LOAD_OP_CLEAR;
				vkAttachmentDescription.storeOp			= VK_ATTACHMENT_STORE_OP_STORE;
				vkAttachmentDescription.stencilLoadOp	= VK_ATTACHMENT_LOAD_OP_DONT_CARE;
				vkAttachmentDescription.stencilStoreOp	= VK_ATTACHMENT_STORE_OP_STORE;
				vkAttachmentDescription.initialLayout	= VK_IMAGE_LAYOUT_UNDEFINED;
				vkAttachmentDescription.finalLayout		= VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

				hasDepth	= true;
				hasStencil	= true;
			}
			else if (attachment.type == VULKAN_ATTACHMENT_TYPE_STENCIL)
			{
				vkAttachmentDescription.loadOp			= VK_ATTACHMENT_LOAD_OP_DONT_CARE;
				vkAttachmentDescription.storeOp			= VK_ATTACHMENT_STORE_OP_DONT_CARE;
				vkAttachmentDescription.stencilLoadOp	= VK_ATTACHMENT_LOAD_OP_DONT_CARE;
				vkAttachmentDescription.stencilStoreOp	= VK_ATTACHMENT_STORE_OP_STORE;
				vkAttachmentDescription.initialLayout	= VK_IMAGE_LAYOUT_UNDEFINED;
				vkAttachmentDescription.finalLayout		= VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL;

				hasStencil = true;
			}
			else
			{
				vkAttachmentDescription.loadOp			= VK_ATTACHMENT_LOAD_OP_CLEAR;
				vkAttachmentDescription.storeOp			= VK_ATTACHMENT_STORE_OP_STORE;
				vkAttachmentDescription.stencilLoadOp	= VK_ATTACHMENT_LOAD_OP_DONT_CARE;
				vkAttachmentDescription.stencilStoreOp	= VK_ATTACHMENT_STORE_OP_DONT_CARE;
				vkAttachmentDescription.initialLayout	= VK_IMAGE_LAYOUT_UNDEFINED;
				vkAttachmentDescription.finalLayout		= VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

				hasColor = true;
			}

			attachmentDescriptions.PushBack(vkAttachmentDescription);
		}

		uInt32 subpassIndex = 0;
		for (const VulkanSubpass& subpass : info.subpasses)
		{
			Array<VkAttachmentReference>&	inputReferences			= subpassInputAttachmentReferences[subpassIndex];
			Array<VkAttachmentReference>&	colorReferences			= subpassColorAttachmentReferences[subpassIndex];
			VkAttachmentReference&			depthStencilReference	= subpassDepthAttachmentReferences[subpassIndex];

			for (uInt32 attachmentId : subpass.attachments)
			{
				// @TODO: index check!!!
				const VulkanAttachment& attachment = info.attachments[attachmentId];

				VkAttachmentReference vkAttachmentReference = {};
				vkAttachmentReference.attachment = attachmentId;

				if (attachment.type == VULKAN_ATTACHMENT_TYPE_SWAPCHAIN || 
					attachment.type == VULKAN_ATTACHMENT_TYPE_COLOR)
				{
					vkAttachmentReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
					colorReferences.PushBack(vkAttachmentReference);
				}
				else if (attachment.type == VULKAN_ATTACHMENT_TYPE_DEPTH ||
					attachment.type == VULKAN_ATTACHMENT_TYPE_STENCIL ||
					attachment.type == VULKAN_ATTACHMENT_TYPE_DEPTH_STENCIL)
				{
					// @TODO: check for more than one depth/stencil
					vkAttachmentReference.layout = attachmentDescriptions[attachmentId].finalLayout;
					depthStencilReference = vkAttachmentReference;
				}
			}

			VkSubpassDescription vkSubpassDescription = {};
			vkSubpassDescription.flags						= 0;
			vkSubpassDescription.pipelineBindPoint			= VK_PIPELINE_BIND_POINT_GRAPHICS;
			vkSubpassDescription.inputAttachmentCount		= 0;
			vkSubpassDescription.pInputAttachments			= nullptr;
			vkSubpassDescription.colorAttachmentCount		= colorReferences.Size();
			vkSubpassDescription.pColorAttachments			= hasColor ? colorReferences.Data() : nullptr;
			vkSubpassDescription.pResolveAttachments		= 0;
			vkSubpassDescription.pDepthStencilAttachment	= hasDepth || hasStencil ? &depthStencilReference : nullptr;
			vkSubpassDescription.preserveAttachmentCount	= 0;
			vkSubpassDescription.pPreserveAttachments		= nullptr;

			subpassDescriptions.PushBack(vkSubpassDescription);
			
			VkSubpassDependency vkSubpassDependancy = {};
			vkSubpassDependancy.srcSubpass		= (subpassIndex - 1);
			vkSubpassDependancy.dstSubpass		= subpassIndex;
			vkSubpassDependancy.srcStageMask	= VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			vkSubpassDependancy.dstStageMask	= VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
			vkSubpassDependancy.srcAccessMask	= VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
			vkSubpassDependancy.dstAccessMask	= VK_ACCESS_SHADER_READ_BIT;
			vkSubpassDependancy.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
			
			if (subpassIndex == 0)
			{
				vkSubpassDependancy.srcSubpass		= VK_SUBPASS_EXTERNAL;
				vkSubpassDependancy.srcStageMask	= VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
				vkSubpassDependancy.dstStageMask	= VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
				vkSubpassDependancy.srcAccessMask	= VK_ACCESS_MEMORY_READ_BIT;
				vkSubpassDependancy.dstAccessMask	= VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
			}

			if (subpassIndex == info.subpasses.Size())
			{
				vkSubpassDependancy.dstSubpass		= VK_SUBPASS_EXTERNAL;
				vkSubpassDependancy.dstStageMask	= VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
				vkSubpassDependancy.srcAccessMask	= VK_ACCESS_MEMORY_READ_BIT;
				vkSubpassDependancy.dstAccessMask	= VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
			}

			subpassDependancies.PushBack(vkSubpassDependancy);

			subpassIndex++;
		}
		
		VkRenderPassCreateInfo vkRenderPassCreateInfo = {};
		vkRenderPassCreateInfo.sType			= VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		vkRenderPassCreateInfo.flags			= 0;
		vkRenderPassCreateInfo.pNext			= nullptr;
		vkRenderPassCreateInfo.attachmentCount	= attachmentDescriptions.Size();
		vkRenderPassCreateInfo.pAttachments		= attachmentDescriptions.Data();
		vkRenderPassCreateInfo.subpassCount		= subpassDescriptions.Size();
		vkRenderPassCreateInfo.pSubpasses		= subpassDescriptions.Data();
		vkRenderPassCreateInfo.dependencyCount	= subpassDependancies.Size();
		vkRenderPassCreateInfo.pDependencies	= subpassDependancies.Data();

		if (vkCreateRenderPass(device.vkDevice, &vkRenderPassCreateInfo, nullptr, &vkRenderPass) != VK_SUCCESS)
		{
			LogError("Failed to create vulkan render pass: vkCreateRenderPass failed!");
			return nullptr;
		}

		LogTrace("Created VulkanRenderpass [ID=%06.6d].", mRenderpasss.Size() + 1);

		VulkanRenderpass vulkanRenderpass = {};
		vulkanRenderpass.name			= info.name;
		vulkanRenderpass.vkRenderpass	= vkRenderPass;
		vulkanRenderpass.vkInfo			= vkRenderPassCreateInfo;

		return Register(vulkanRenderpass);
	}

	VulkanGraphicsPipeline* VulkanResourceManager::CreateGraphicsPipeline(VulkanDevice& device, const VulkanGraphicsPipelineInfo& info, uInt32 subpass)
	{
		VkPipeline vkPipeline = VK_NULL_HANDLE;

		Array<VkPipelineShaderStageCreateInfo>				shaderStageInfos;
		Array<VulkanDescriptorSetInfo>						descriptorSetInfos;
		Array<Map<String, VkDescriptorSetLayoutBinding>>	descriptorSetBindings(16); // @Todo: querry max supported sets
		Array<uInt32>										descriptorSetSizes(16);
		Array<VkDescriptorSetLayout>						descriptorSetLayouts;
		
		uSize maxSetIndex = 0;

		for (VulkanShader* pShader : info.shaders)
		{
			if (!pShader) continue;

			VulkanShader* pVulkanShader = static_cast<VulkanShader*>(pShader);

			VkPipelineShaderStageCreateInfo vkShaderStageInfo = {};
			vkShaderStageInfo.sType					= VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
			vkShaderStageInfo.flags					= 0;
			vkShaderStageInfo.pSpecializationInfo	= nullptr;
			vkShaderStageInfo.stage					= pVulkanShader->vkStage;
			vkShaderStageInfo.module				= pVulkanShader->vkShader;
			vkShaderStageInfo.pName					= pVulkanShader->entryPoint.Str();
			vkShaderStageInfo.pNext					= nullptr;

			shaderStageInfos.PushBack(vkShaderStageInfo);

			const Array<SpirvUniform>& uniforms = pVulkanShader->uniforms;

			for (uInt32 i = 0; i < uniforms.Size(); i++)
			{
				const SpirvUniform& uniform = uniforms[i];

				if (descriptorSetBindings[uniform.set].Contains(uniform.name))
				{
					descriptorSetBindings[uniform.set][uniform.name].stageFlags |= pVulkanShader->vkStage;
				}
				else
				{
					VkDescriptorSetLayoutBinding vkBinding;
					vkBinding.binding				= uniform.binding;
					vkBinding.descriptorCount		= 1;
					vkBinding.descriptorType		= uniform.descriptorType;
					vkBinding.stageFlags			= pVulkanShader->vkStage;
					vkBinding.pImmutableSamplers	= nullptr;

					// @TODO: check set bounds < 16
					descriptorSetBindings[uniform.set].Put(uniform.name, vkBinding);
					descriptorSetSizes[uniform.set] += uniform.sizeBytes;
				}

				if (uniform.set > maxSetIndex)
				{
					maxSetIndex = uniform.set;
				}
			}

			// @Todo: samplers, etc.
		}

		descriptorSetLayouts.Resize(maxSetIndex + 1);

		uInt32 setIndex = 0;
		for (Map<String, VkDescriptorSetLayoutBinding>& set : descriptorSetBindings)
		{
			if (setIndex > maxSetIndex)
			{
				break;
			}

			Array<VkDescriptorSetLayoutBinding> setBindings;

			for (MapPair<String, VkDescriptorSetLayoutBinding>& binding : set)
			{
				setBindings.PushBack(binding.value);
			}

			VkDescriptorSetLayoutCreateInfo vkDescriptorSetLayoutInfo = {};
			vkDescriptorSetLayoutInfo.sType			= VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
			vkDescriptorSetLayoutInfo.flags			= 0;
			vkDescriptorSetLayoutInfo.bindingCount	= setBindings.Size();
			vkDescriptorSetLayoutInfo.pBindings		= setBindings.Data();
			vkDescriptorSetLayoutInfo.pNext			= nullptr; //&bindingFlags;

			VkDescriptorSetLayout vkDescriptorSetLayout;

			if (vkCreateDescriptorSetLayout(device.vkDevice, &vkDescriptorSetLayoutInfo, nullptr, &vkDescriptorSetLayout) != VK_SUCCESS)
			{
				LogError("Failed to create vulkan descriptor set layout: vkCreateDescriptorSetLayout failed!");
				// @TODO: Destroy other sets
				return nullptr;
			}

			descriptorSetLayouts[setIndex] = vkDescriptorSetLayout;

			VulkanDescriptorSetInfo descriptorSetInfo;
			descriptorSetInfo.set					= setIndex;
			descriptorSetInfo.vkDescriptorSetLayout = vkDescriptorSetLayout;
			descriptorSetInfo.sizeBytes				= descriptorSetSizes[setIndex];

			for (const MapPair<String, VkDescriptorSetLayoutBinding>& binding : set)
			{
				descriptorSetInfo.bindings.PushBack(binding.value);
			}

			descriptorSetInfos.PushBack(descriptorSetInfo);

			setIndex++;
		}

		VkPipelineVertexInputStateCreateInfo vkVertexInputStateInfo = {};
		vkVertexInputStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		vkVertexInputStateInfo.vertexBindingDescriptionCount	= info.bufferAttachments.Size();
		vkVertexInputStateInfo.pVertexBindingDescriptions		= info.bufferAttachments.Data();
		vkVertexInputStateInfo.vertexAttributeDescriptionCount	= info.vertexAttributes.Size();
		vkVertexInputStateInfo.pVertexAttributeDescriptions		= info.vertexAttributes.Data();
		vkVertexInputStateInfo.flags = 0;

		VkPipelineInputAssemblyStateCreateInfo vkInputAssemblyInfo = {};
		vkInputAssemblyInfo.sType		= VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		vkInputAssemblyInfo.topology	= info.vkTopology;
		vkInputAssemblyInfo.flags		= 0;

		VkPipelineViewportStateCreateInfo vkViewportInfo = {};
		vkViewportInfo.sType			= VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		vkViewportInfo.viewportCount	= 1;
		vkViewportInfo.pViewports		= &info.viewport;
		vkViewportInfo.scissorCount		= 1;
		vkViewportInfo.pScissors		= &info.scissor;
		vkViewportInfo.flags			= 0;

		VkPipelineRasterizationStateCreateInfo vkRasterizationInfo = {};
		vkRasterizationInfo.sType					= VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		vkRasterizationInfo.flags					= 0;
		vkRasterizationInfo.depthClampEnable		= VK_FALSE;
		vkRasterizationInfo.rasterizerDiscardEnable = VK_FALSE;
		vkRasterizationInfo.polygonMode				= info.vkPolygonMode;
		vkRasterizationInfo.cullMode				= info.vkCullMode;
		vkRasterizationInfo.frontFace				= info.vkFrontFace;
		vkRasterizationInfo.depthBiasEnable			= VK_FALSE;
		vkRasterizationInfo.depthBiasConstantFactor = 0;
		vkRasterizationInfo.depthBiasClamp			= 0;
		vkRasterizationInfo.depthBiasSlopeFactor	= 0;
		vkRasterizationInfo.lineWidth				= info.lineWidth;

		uInt32 maxMultisamples = device.pPhysicalDevice->vkProperties.limits.framebufferColorSampleCounts;
		VkSampleCountFlagBits validMultisamples = info.multisamples;

		if (validMultisamples == 0)
		{
			validMultisamples = VK_SAMPLE_COUNT_1_BIT;
			LogWarning("Invalid zero multisample value in pipeline creation, using multisamples value of 1");
		}

		if (validMultisamples > maxMultisamples)
		{
			validMultisamples = (VkSampleCountFlagBits)maxMultisamples;
			LogWarning("Invalid maximum multisample value in pipeline creation, using max multisample value of %d", maxMultisamples);
		}

		VkPipelineMultisampleStateCreateInfo vkMultisampleInfo = {};
		vkMultisampleInfo.sType					= VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		vkMultisampleInfo.flags					= 0;
		vkMultisampleInfo.rasterizationSamples	= validMultisamples;
		vkMultisampleInfo.sampleShadingEnable	= VK_FALSE;
		vkMultisampleInfo.minSampleShading		= 0.0f;
		vkMultisampleInfo.pSampleMask			= nullptr;
		vkMultisampleInfo.alphaToCoverageEnable = VK_FALSE;
		vkMultisampleInfo.alphaToOneEnable		= VK_FALSE;

		VkPipelineDepthStencilStateCreateInfo vkDepthStencilInfo = {};
		vkDepthStencilInfo.sType					= VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		vkDepthStencilInfo.flags					= 0;
		vkDepthStencilInfo.depthTestEnable			= info.depth.enableTesting;
		vkDepthStencilInfo.depthWriteEnable			= info.depth.enableWrite;
		vkDepthStencilInfo.depthCompareOp			= info.depth.compareOp;
		vkDepthStencilInfo.depthBoundsTestEnable	= VK_FALSE;
		vkDepthStencilInfo.stencilTestEnable		= info.stencil.enableTesting;
		vkDepthStencilInfo.front.failOp				= VK_STENCIL_OP_KEEP;
		vkDepthStencilInfo.front.passOp				= VK_STENCIL_OP_KEEP;
		vkDepthStencilInfo.front.depthFailOp		= VK_STENCIL_OP_KEEP;
		vkDepthStencilInfo.front.compareOp			= info.stencil.compareOp;
		vkDepthStencilInfo.front.compareMask		= 0;
		vkDepthStencilInfo.front.writeMask			= 0;
		vkDepthStencilInfo.front.reference			= 0;
		vkDepthStencilInfo.back.failOp				= VK_STENCIL_OP_KEEP;
		vkDepthStencilInfo.back.passOp				= VK_STENCIL_OP_KEEP;
		vkDepthStencilInfo.back.depthFailOp			= VK_STENCIL_OP_KEEP;
		vkDepthStencilInfo.back.compareOp			= info.stencil.compareOp;
		vkDepthStencilInfo.back.compareMask			= 0;
		vkDepthStencilInfo.back.writeMask			= 0;
		vkDepthStencilInfo.back.reference			= 0;
		vkDepthStencilInfo.minDepthBounds			= 0.0f;
		vkDepthStencilInfo.maxDepthBounds			= 1.0f;

		VkPipelineColorBlendStateCreateInfo vkColorBlendInfo = {};
		vkColorBlendInfo.sType				= VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		vkColorBlendInfo.flags				= 0;
		vkColorBlendInfo.logicOpEnable		= VK_FALSE;
		vkColorBlendInfo.logicOp			= VK_LOGIC_OP_COPY;
		vkColorBlendInfo.attachmentCount	= info.blendAttachments.Size();
		vkColorBlendInfo.pAttachments		= info.blendAttachments.Size() ? info.blendAttachments.Data() : nullptr;
		vkColorBlendInfo.blendConstants[0]	= 0.0f;//1.0f
		vkColorBlendInfo.blendConstants[1]	= 0.0f;//1.0f
		vkColorBlendInfo.blendConstants[2]	= 0.0f;//1.0f
		vkColorBlendInfo.blendConstants[3]	= 0.0f;//1.0f

		VkPipelineLayout vkPipelineLayout;

		VkPipelineLayoutCreateInfo vkLayoutInfo = {};
		vkLayoutInfo.sType			= VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		vkLayoutInfo.flags			= 0;
		vkLayoutInfo.setLayoutCount = descriptorSetLayouts.Size();
		vkLayoutInfo.pSetLayouts	= descriptorSetLayouts.Data();

		// TODO: Support push constants?
		vkLayoutInfo.pushConstantRangeCount = 0;
		vkLayoutInfo.pPushConstantRanges = nullptr;

		if (vkCreatePipelineLayout(device.vkDevice, &vkLayoutInfo, nullptr, &vkPipelineLayout) != VK_SUCCESS)
		{
			LogError("Failed to create vulkan pipeline layout: vkCreatePipelineLayout failed!");
			// @TODO: Destroy all descriptorSets
			return nullptr;
		}

		///////////////////////////////////////////////////////////////

		VkGraphicsPipelineCreateInfo vkPipelineInfo = {};
		vkPipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;

		bool parent = false;
		if (parent)
		{
			vkPipelineInfo.flags				|= VK_PIPELINE_CREATE_DERIVATIVE_BIT;
			vkPipelineInfo.basePipelineHandle	= 0;//parentId;
			vkPipelineInfo.basePipelineIndex	= 0;
		}

		vkPipelineInfo.stageCount			= shaderStageInfos.Size();
		vkPipelineInfo.pStages				= shaderStageInfos.Data();
		vkPipelineInfo.pVertexInputState	= &vkVertexInputStateInfo;
		vkPipelineInfo.pInputAssemblyState	= &vkInputAssemblyInfo;
		vkPipelineInfo.pTessellationState	= nullptr;
		vkPipelineInfo.pViewportState		= &vkViewportInfo;
		vkPipelineInfo.pRasterizationState	= &vkRasterizationInfo;
		vkPipelineInfo.pMultisampleState	= &vkMultisampleInfo;
		vkPipelineInfo.pDepthStencilState = nullptr;//&vkDepthStencilInfo;
		vkPipelineInfo.pColorBlendState		= &vkColorBlendInfo;
		vkPipelineInfo.pDynamicState		= nullptr; //TODO
		vkPipelineInfo.layout				= vkPipelineLayout;
		vkPipelineInfo.renderPass			= info.pRenderpass->vkRenderpass;
		vkPipelineInfo.subpass				= subpass;

		if (vkCreateGraphicsPipelines(device.vkDevice, VK_NULL_HANDLE, 1, &vkPipelineInfo, nullptr, &vkPipeline) != VK_SUCCESS)
		{
			LogError("Failed to create vulkan graphics pipeline: vkCreateGraphicsPipelines failed!");
			// @TODO: Destroy everything
			return nullptr;
		}

		LogTrace("Created VulkanGraphicsPipeline [ID=%06.6d].", mGraphicsPipelines.Size() + 1);

		VulkanGraphicsPipeline vulkanGraphicsPipeline = {};
		vulkanGraphicsPipeline.pDevice			= &device;
		vulkanGraphicsPipeline.vkPipeline		= vkPipeline;
		vulkanGraphicsPipeline.vkPipelineInfo	= vkPipelineInfo;

		return Register(vulkanGraphicsPipeline);
	}

	uInt32 FindCompatableMemoryType(VulkanDevice& device, flags32 memoryTypeBits, VkMemoryPropertyFlags memoryProperties)
	{
		VkPhysicalDeviceMemoryProperties deviceMemoryProperties = device.pPhysicalDevice->vkMemoryProperties;

		for (uInt32 i = 0; i < deviceMemoryProperties.memoryTypeCount; i++)
		{
			if ((memoryTypeBits & (1 << i)) && (deviceMemoryProperties.memoryTypes[i].propertyFlags & memoryProperties) == memoryProperties)
			{
				return i;
			}
		}

		return (uInt32)-1;
	}

	VulkanBuffer* VulkanResourceManager::CreateBuffer(VulkanDevice& device, const VulkanBufferInfo& info)
	{
		VkBuffer				vkBuffer;
		VkDeviceMemory			vkMemory;
		VkMemoryRequirements	vkMemRequirements;

		VkBufferCreateInfo bufferInfo = {};
		bufferInfo.sType		= VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferInfo.size			= info.sizeBytes;
		bufferInfo.usage		= info.vkBufferUsage;
		bufferInfo.sharingMode	= VK_SHARING_MODE_EXCLUSIVE;

		if (vkCreateBuffer(device.vkDevice, &bufferInfo, nullptr, &vkBuffer) != VK_SUCCESS)
		{
			LogError("Failed to create vulkan buffer object: vkCreateBuffer failed!");
			return nullptr;
		}

		vkGetBufferMemoryRequirements(device.vkDevice, vkBuffer, &vkMemRequirements);

		if (vkMemRequirements.size < info.sizeBytes)
		{
			LogError("Failed to create vulkan buffer object: Requested of %d bytes exceeds the maximum of %d bytes.", 
				info.sizeBytes, vkMemRequirements.size);
			return nullptr;
		}

		uInt32 memoryType = FindCompatableMemoryType(device, vkMemRequirements.memoryTypeBits, info.vkMemoryProperties);

		VkMemoryAllocateInfo allocateInfo = {};
		allocateInfo.sType				= VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocateInfo.allocationSize		= vkMemRequirements.size;
		allocateInfo.memoryTypeIndex	= memoryType;

		if (vkAllocateMemory(device.vkDevice, &allocateInfo, nullptr, &vkMemory) != VK_SUCCESS)
		{
			LogError("Failed to create vulkan buffer object: vkAllocateMemory failed!");
			vkDestroyBuffer(device.vkDevice, vkBuffer, VK_NULL_HANDLE);
			return nullptr;
		}

		if (vkBindBufferMemory(device.vkDevice, vkBuffer, vkMemory, 0) != VK_SUCCESS)
		{
			LogError("Failed to create vulkan buffer object: vkBindBufferMemory failed!");

			vkDestroyBuffer(device.vkDevice, vkBuffer, VK_NULL_HANDLE);
			vkFreeMemory(device.vkDevice, vkMemory, VK_NULL_HANDLE);

			return nullptr;
		}

		LogTrace("Created VulkanBuffer [ID=%06.6d, Size=%d bytes].", mBuffers.Size() + 1, info.sizeBytes);

		VulkanBuffer vulkanBuffer = {};
		vulkanBuffer.vkBuffer			= vkBuffer;
		vulkanBuffer.pDevice			= &device;
		vulkanBuffer.sizeBytes			= info.sizeBytes;
		vulkanBuffer.vkMemory			= vkMemory;
		vulkanBuffer.vkMemoryProperties	= info.vkMemoryProperties;
		vulkanBuffer.vkUsage			= info.vkBufferUsage;

		return Register(vulkanBuffer);
	}

	VulkanCommandPool* VulkanResourceManager::CreateCommandPool(VulkanDevice& device, const VulkanCommandPoolInfo& info)
	{
		VkCommandPool vkCommandPool;

		VkCommandPoolCreateInfo commandPoolInfo = {};
		commandPoolInfo.sType				= VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		commandPoolInfo.queueFamilyIndex	= info.queueFamilyIndex;
		commandPoolInfo.flags				= info.vkCommandPoolCreateFlags;

		if (vkCreateCommandPool(device.vkDevice, &commandPoolInfo, nullptr, &vkCommandPool) != VK_SUCCESS)
		{
			LogError("Failed to create VulkanCommandPool: vkCreateCommandPool failed!");
			return nullptr;
		}

		LogTrace("Created VulkanCommandPool [ID=%06.6d].", mCommandPools.Size() + 1);

		VulkanCommandPool vulkanCommandPool = {};
		vulkanCommandPool.vkCommandPool				= vkCommandPool;
		vulkanCommandPool.pDevice					= &device;
		vulkanCommandPool.queueFamilyIndex			= info.queueFamilyIndex;
		vulkanCommandPool.vkCommandPoolCreateFlags	= info.vkCommandPoolCreateFlags;

		return Register(vulkanCommandPool);
	}

	bool VulkanResourceManager::CreateCommandBuffers(VulkanCommandPool* pCommandPool, uInt32 count, VulkanCommandBuffer** ppCommandBuffers)
	{
		Array<VkCommandBuffer> vkCommandBuffersList;
		vkCommandBuffersList.Resize(count);

		VkCommandBufferAllocateInfo allocInfo = {};
		allocInfo.sType					= VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.commandPool			= pCommandPool->vkCommandPool;
		allocInfo.level					= VK_COMMAND_BUFFER_LEVEL_PRIMARY; // Secondary buffers not worth it
		allocInfo.commandBufferCount	= count;

		if (vkAllocateCommandBuffers(pCommandPool->pDevice->vkDevice, &allocInfo, vkCommandBuffersList.Data()) != VK_SUCCESS)
		{
			LogError("Failed to create VulkanCommandBuffer(s): vkAllocateCommandBuffers failed!");
			return false;
		}

		for (uSize i = 0; i < count; i++)
		{
			VulkanCommandBuffer vulkanCommandBuffer = {};
			vulkanCommandBuffer.vkCommandBuffer = vkCommandBuffersList[i];
			vulkanCommandBuffer.pCommandPool	= pCommandPool;
			vulkanCommandBuffer.pDevice			= pCommandPool->pDevice;

			LogTrace("Created VulkanCommandBuffer [ID=%06.6d].", mCommandBuffers.Size() + 1);

			ppCommandBuffers[i] = Register(vulkanCommandBuffer);
		}
	}

	VulkanFramebuffer* VulkanResourceManager::CreateFramebuffer(VulkanDevice& device, const VulkanFramebufferInfo& info)
	{
		VkFramebuffer vkFramebuffer;

		constexpr const uSize attachmentImageViewsSize = 16;

		VkImageView vkAttachmentImageViews[attachmentImageViewsSize];

		for (uSize i = 0; i < info.attachments.Size(); i++)
		{
			vkAttachmentImageViews[i] = info.attachments[i]->vkImageView;
		}

		VkFramebufferCreateInfo framebufferInfo = {};
		framebufferInfo.sType			= VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferInfo.renderPass		= info.renderpass->vkRenderpass;
		framebufferInfo.attachmentCount = info.attachments.Size();
		framebufferInfo.pAttachments	= vkAttachmentImageViews;
		framebufferInfo.width			= info.width;
		framebufferInfo.height			= info.height;
		framebufferInfo.layers			= info.layers;

		if (vkCreateFramebuffer(device.vkDevice, &framebufferInfo, VK_NULL_HANDLE, &vkFramebuffer) != VK_SUCCESS)
		{
			LogError("Failed to create VulkanFramebuffer: vkCreateFramebuffer failed!");
		}

		VulkanFramebuffer vulkanFramebuffer = {};
		vulkanFramebuffer.vkFramebuffer		= vkFramebuffer;
		vulkanFramebuffer.renderpass		= info.renderpass;
		vulkanFramebuffer.attachments		= info.attachments;
		vulkanFramebuffer.width				= info.width;
		vulkanFramebuffer.height			= info.height;
		vulkanFramebuffer.layers			= info.layers;

		LogTrace("Created VulkanFramebuffer [ID=%06.6d].", mFramebuffers.Size() + 1);

		return Register(vulkanFramebuffer);
	}
}