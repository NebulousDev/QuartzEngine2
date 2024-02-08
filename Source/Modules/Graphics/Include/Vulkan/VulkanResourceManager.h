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
		VulkanSurface*			CreateSurface(VulkanDevice* pDevice, VkInstance vkInstance, const VulkanApiSurface& surface);
		VulkanSwapchain*		CreateSwapchain(VulkanDevice* pDevice, const VulkanSurface& surface, uInt32 bufferCount);
		VulkanImage*			CreateImage(VulkanDevice* pDevice, const VulkanImageInfo& info);
		VulkanImageView*		CreateImageView(VulkanDevice* pDevice, const VulkanImageViewInfo& info);
		VulkanShader*			CreateShader(VulkanDevice* pDevice, const String& name, const Array<uInt8>& binary);
		VulkanRenderpass*		CreateRenderpass(VulkanDevice* pDevice, const VulkanRenderpassInfo& info);
		VulkanGraphicsPipeline* CreateGraphicsPipeline(VulkanDevice* pDevice, const VulkanGraphicsPipelineInfo& info, uInt32 subpass);
		VulkanBuffer*			CreateBuffer(VulkanDevice* pDevice, const VulkanBufferInfo& info);
		VulkanCommandPool*		CreateCommandPool(VulkanDevice* pDevice, const VulkanCommandPoolInfo& info);
		bool					CreateCommandBuffers(VulkanCommandPool* pCommandPool, uInt32 count, VulkanCommandBuffer** ppCommandBuffers);
		VulkanFramebuffer*		CreateFramebuffer(VulkanDevice* pDevice, const VulkanFramebufferInfo& info);
	};
}