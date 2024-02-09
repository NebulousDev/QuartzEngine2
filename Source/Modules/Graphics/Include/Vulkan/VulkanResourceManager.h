#pragma once

#include "../GfxDLL.h"
#include "Types/Array.h"

#include "Primatives/VulkanSurface.h"
#include "Primatives/VulkanSwapchain.h"
#include "Primatives/VulkanImage.h"
#include "Primatives/VulkanShader.h"
#include "Primatives/VulkanRenderpass.h"
#include "Primatives/VulkanPipeline.h"
#include "Primatives/VulkanBuffer.h"
#include "Primatives/VulkanCommandBuffer.h"
#include "Primatives/VulkanFramebuffer.h"
#include "Primatives/VulkanDescriptorSet.h"
#include "Primatives/VulkanDescriptorSetLayout.h"

#include <vulkan/vulkan.h>

namespace Quartz
{
	class VulkanDevice;
	class VulkanApiSurface;

	class QUARTZ_GRAPHICS_API VulkanResourceManager
	{
	private:
		Array<VulkanSurface>				mSurfaces;
		Array<VulkanSwapchain>				mSwapchains;
		Array<VulkanImage>					mImages;
		Array<VulkanImageView>				mImageViews;
		Array<VulkanShader>					mShaders;
		Array<VulkanRenderpass>				mRenderpasss;
		Array<VulkanGraphicsPipeline>		mGraphicsPipelines;
		Array<VulkanBuffer>					mBuffers;
		Array<VulkanCommandPool>			mCommandPools;
		Array<VulkanCommandBuffer>			mCommandBuffers;
		Array<VulkanFramebuffer>			mFramebuffers;
		Array<VulkanDescriptorPool>			mDescriptorPools;
		Array<VulkanDescriptorSet>			mDescriptorSets;
		Array<VulkanDescriptorSetLayout>	mDescriptorSetLayouts;

		VulkanSurface*				Register(const VulkanSurface& surface);
		VulkanSwapchain*			Register(const VulkanSwapchain& swapchain);
		VulkanImage*				Register(const VulkanImage& image);
		VulkanImageView*			Register(const VulkanImageView& imageView);
		VulkanShader*				Register(const VulkanShader& shader);
		VulkanRenderpass*			Register(const VulkanRenderpass& renderpass);
		VulkanGraphicsPipeline*		Register(const VulkanGraphicsPipeline& pipeline);
		VulkanBuffer*				Register(const VulkanBuffer& buffer);
		VulkanCommandPool*			Register(const VulkanCommandPool& pool);
		VulkanCommandBuffer*		Register(const VulkanCommandBuffer& buffer);
		VulkanFramebuffer*			Register(const VulkanFramebuffer& framebuffer);
		VulkanDescriptorPool*		Register(const VulkanDescriptorPool& pool);
		VulkanDescriptorSet*		Register(const VulkanDescriptorSet& set);
		VulkanDescriptorSetLayout*	Register(const VulkanDescriptorSetLayout& layout);

	public:
		VulkanSurface*				CreateSurface(VulkanDevice* pDevice, VkInstance vkInstance, const VulkanApiSurface& surface);
		VulkanSwapchain*			CreateSwapchain(VulkanDevice* pDevice, const VulkanSurface& surface, uInt32 bufferCount);
		VulkanImage*				CreateImage(VulkanDevice* pDevice, const VulkanImageInfo& info);
		VulkanImageView*			CreateImageView(VulkanDevice* pDevice, const VulkanImageViewInfo& info);
		VulkanShader*				CreateShader(VulkanDevice* pDevice, const String& name, const Array<uInt8>& binary);
		VulkanRenderpass*			CreateRenderpass(VulkanDevice* pDevice, const VulkanRenderpassInfo& info);
		VulkanGraphicsPipeline*		CreateGraphicsPipeline(VulkanDevice* pDevice, const VulkanGraphicsPipelineInfo& info, uInt32 subpass);
		VulkanBuffer*				CreateBuffer(VulkanDevice* pDevice, const VulkanBufferInfo& info);
		VulkanCommandPool*			CreateCommandPool(VulkanDevice* pDevice, const VulkanCommandPoolInfo& info);
		bool						CreateCommandBuffers(VulkanCommandPool* pCommandPool, uInt32 count, VulkanCommandBuffer** ppOutCommandBuffers);
		VulkanFramebuffer*			CreateFramebuffer(VulkanDevice* pDevice, const VulkanFramebufferInfo& info);
		VulkanDescriptorPool*		CreateDescriptorPool(VulkanDevice* pDevice, const VulkanDescriptorPoolInfo& info);
		bool						CreateDescriptorSets(VulkanDevice* pDevice, const VulkanDescriptorSetAllocationInfo& info, 
										VulkanDescriptorSet** ppOutDescriptorSets);
		VulkanDescriptorSetLayout*	CreateDescriptorSetLayout(VulkanDevice* pDevice, const VulkanDesctiptorSetLayoutInfo& info);
	};
}