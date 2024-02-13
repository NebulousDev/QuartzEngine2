#pragma once

#include "../GfxAPI.h"
#include "Runtime/Runtime.h"

#include "VulkanGraphics.h"
#include "VulkanRenderScene.h"
#include "VulkanSwapchainTimer.h"
#include "VulkanShaderCache.h"
#include "VulkanPipelineCache.h"
#include "VulkanBufferWriter.h"

#include "Math/Math.h"

namespace Quartz
{
	class QUARTZ_GRAPHICS_API VulkanRenderer
	{
	private:
		VulkanGraphics*			mpGraphics;
		VulkanSwapchain*		mpSwapchain;
		VulkanRenderScene		mRenderScene;
		VulkanShaderCache		mShaderCache;
		VulkanPipelineCache		mPipelineCache;
		VulkanSwapchainTimer	mSwapTimer;

		VulkanGraphicsPipeline* mpPipeline;

		VulkanCommandBuffer*	mCommandBuffers[3];
		VulkanImage*			mDepthImages[3];
		VulkanImageView*		mDepthImageViews[3];

	public:
		void Initialize(VulkanGraphics* pGraphics);

		void RenderScene(VulkanRenderScene* pRenderScene);

		void RenderUpdate(Runtime* pRuntime, double delta);
		void Register(Runtime* pRuntime);
	};
}