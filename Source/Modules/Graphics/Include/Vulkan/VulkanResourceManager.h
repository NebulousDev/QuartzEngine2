#pragma once

#include "../GfxDLL.h"
#include "Types/Array.h"

#include "VulkanSurface.h"
#include "VulkanSwapchain.h"
#include "VulkanImage.h"
#include "VulkanShader.h"
#include "VulkanRenderpass.h"
#include "VulkanPipeline.h"
#include "VulkanBuffer.h"
#include "VulkanCommandBuffer.h"
#include "VulkanFramebuffer.h"

#include <vulkan/vulkan.h>

namespace Quartz
{
	class VulkanDevice;
	class VulkanApiSurface;

	class QUARTZ_GRAPHICS_API VulkanResourceManager
	{
	private:
		Array<VulkanSurface>			mSurfaces;
		Array<VulkanSwapchain>			mSwapchains;
		Array<VulkanImage>				mImages;
		Array<VulkanImageView>			mImageViews;
		Array<VulkanShader>				mShaders;
		Array<VulkanRenderpass>			mRenderpasss;
		Array<VulkanGraphicsPipeline>	mGraphicsPipelines;
		Array<VulkanBuffer>				mBuffers;
		Array<VulkanCommandPool>		mCommandPools;
		Array<VulkanCommandBuffer>		mCommandBuffers;
		Array<VulkanFramebuffer>		mFramebuffers;

		VulkanSurface*			Register(const VulkanSurface& surface);
		VulkanSwapchain*		Register(const VulkanSwapchain& swapchain);
		VulkanImage*			Register(const VulkanImage& image);
		VulkanImageView*		Register(const VulkanImageView& imageView);
		VulkanShader*			Register(const VulkanShader& shader);
		VulkanRenderpass*		Register(const VulkanRenderpass& renderpass);
		VulkanGraphicsPipeline* Register(const VulkanGraphicsPipeline& pipeline);
		VulkanBuffer*			Register(const VulkanBuffer& buffer);
		VulkanCommandPool*		Register(const VulkanCommandPool& pool);
		VulkanCommandBuffer*	Register(const VulkanCommandBuffer& buffer);
		VulkanFramebuffer*		Register(const VulkanFramebuffer& framebuffer);

	public:
		VulkanSurface*			CreateSurface(VkInstance vkInstance, const VulkanDevice& device, const VulkanApiSurface& surface);
		VulkanSwapchain*		CreateSwapchain(VulkanDevice& device, const VulkanSurface& surface, uInt32 bufferCount);
		VulkanImage*			CreateImage(VulkanDevice& device, const VulkanImageInfo& info);
		VulkanImageView*		CreateImageView(const VulkanDevice& device, const VulkanImageViewInfo& info);
		VulkanShader*			CreateShader(const VulkanDevice& device, const String& name, const Array<uInt8>& binary);
		VulkanRenderpass*		CreateRenderpass(const VulkanDevice& device, const VulkanRenderpassInfo& info);
		VulkanGraphicsPipeline* CreateGraphicsPipeline(VulkanDevice& device, const VulkanGraphicsPipelineInfo& info, uInt32 subpass);
		VulkanBuffer*			CreateBuffer(VulkanDevice& device, const VulkanBufferInfo& info);
		VulkanCommandPool*		CreateCommandPool(VulkanDevice& device, const VulkanCommandPoolInfo& info);
		bool					CreateCommandBuffers(VulkanCommandPool* pCommandPool, uInt32 count,
									VulkanCommandBuffer** ppCommandBuffers);
		VulkanFramebuffer*		CreateFramebuffer(VulkanDevice& device, const VulkanFramebufferInfo& info);
	};
}