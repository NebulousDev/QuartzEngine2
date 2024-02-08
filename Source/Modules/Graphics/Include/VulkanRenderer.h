#pragma once

#include "GfxDLL.h"
#include "Runtime/Runtime.h"

#include "Vulkan/VulkanGraphics.h"
#include "Vulkan/VulkanRenderScene.h"
#include "Vulkan/VulkanSwapchainTimer.h"
#include "Vulkan/VulkanPipeline.h"
#include "Vulkan/VulkanBufferWriter.h"

#include "Math/Math.h"

namespace Quartz
{
	struct TransformUniformData
	{
		Mat4f transform;
		Mat4f view;
	};

	class QUARTZ_GRAPHICS_API VulkanRenderer
	{
	private:
		VulkanGraphics*			mpGraphics;
		VulkanSwapchain*		mpSwapchain;
		VulkanGraphicsPipeline* mpPipeline;
		VulkanSwapchainTimer*	mpSwapTimer;

		VulkanCommandBuffer*	mCommandBuffers[3];
		VulkanFramebuffer*		mFramebuffers[3];
		VulkanBuffer*			mUniformTransformStagingBuffers[3];
		VulkanBuffer*			mUniformTransformBuffers[3];
		VulkanBufferWriter		mUniformTransformWriters[3];
		TransformUniformData*	mTransformData[3];
		VulkanDescriptorPool*	mGlobalDescriptorPool;
		VulkanDescriptorSet*	mTransformDescriptorSets[3];

	public:
		void Initialize(VulkanGraphics* pGraphics);

		void RenderScene(VulkanRenderScene* pRenderScene);

		void RenderUpdate(Runtime* pRuntime, double delta);
		void Register(Runtime* pRuntime);
	};
}