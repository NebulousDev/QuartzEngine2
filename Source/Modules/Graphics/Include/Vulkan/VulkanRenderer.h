#pragma once

#include "../GfxAPI.h"
#include "Runtime/Runtime.h"

#include "VulkanGraphics.h"
#include "VulkanBufferCache.h"
#include "VulkanSwapchainTimer.h"
#include "VulkanBufferWriter.h"

#include "Math/Math.h"

namespace Quartz
{
	class QUARTZ_GRAPHICS_API VulkanRenderer
	{
	private:
		VulkanGraphics*			mpGraphics;
		VulkanSwapchain*		mpSwapchain;
		VulkanBufferCache		mBufferCache;
		VulkanShaderCache		mShaderCache;
		VulkanPipelineCache		mPipelineCache;
		VulkanSwapchainTimer	mSwapTimer;

		VulkanGraphicsPipeline* mpDefaultPipeline;

		Array<VulkanRenderable>	mRenderables;
		Array<VulkanRenderable>	mRenderablesSorted;

		VulkanCommandBuffer*	mCommandBuffers[3];
		VulkanImage*			mDepthImages[3];
		VulkanImageView*		mDepthImageViews[3];

	public:
		void Initialize(VulkanGraphics* pGraphics);

		void UpdateAll(EntityWorld* pEntityWorld);
		void WriteCommandBuffer(VulkanCommandRecorder* pRecorder);
		void RenderScene(EntityWorld* pEntityWorld);

		void RenderUpdate(Runtime* pRuntime, double delta);
		void Register(Runtime* pRuntime);
	};
}