#pragma once

#include "../GfxAPI.h"
#include "Runtime/Runtime.h"

#include "VulkanGraphics.h"
#include "VulkanBufferCache.h"
#include "VulkanSwapchainTimer.h"
#include "VulkanBufferWriter.h"

#include "Component/CameraComponent.h"
#include "Component/TransformComponent.h"

#include "Math/Math.h"

#include "../TerrainRenderer.h"
#include "../SkyRenderer.h"

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

		Entity					mCameraEntity;
		//CameraComponent*		mpCameraComponent;
		//TransformComponent*	mpCameraTransformComponent;

		VulkanTerrainRenderer	mTerrainRenderer;
		VulkanSkyRenderer		mSkyRenderer;

	public:
		void Initialize(VulkanGraphics* pGraphics);

		void SetCamera(Entity cameraEntity);

		void UpdateAll(EntityWorld* pWorld, uSize frameIdx);
		void RecordTransfers(VulkanCommandRecorder& recorder, uInt32 frameIdx);
		void RecordDraws(VulkanCommandRecorder& recorder, uInt32 frameIdx);
		void RenderScene(EntityWorld* pWorld, uSize frameIdx);

		void RenderUpdate(Runtime& runtime, double delta);
		void Register(Runtime& runtime);
	};
}