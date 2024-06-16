#pragma once

#include "../GfxAPI.h"
#include "Math/Math.h"
#include "Runtime/Runtime.h"

#include "Window.h"

#include "VulkanGraphics.h"
#include "VulkanBufferCache.h"
#include "VulkanSwapchainTimer.h"
#include "VulkanBufferWriter.h"

#include "Graphics/Renderer.h"
#include "Renderers/VulkanSceneRenderer.h"
#include "Renderers/VulkanTerrainRenderer.h"
#include "Renderers/VulkanSkyRenderer.h"
#include "Renderers/VulkanImGuiRenderer.h"

namespace Quartz
{
	class QUARTZ_GRAPHICS_API VulkanRenderer : public Renderer
	{
	private:
		VulkanGraphics*				mpGraphics;
		Window*						mpWindow;

		VulkanSwapchain*			mpSwapchain;
		VulkanSwapchainTimer		mSwapTimer;

		uInt64						mTargetFPS			= 350;
		double						mCurrentFPS			= 0.0;
		double						mAverageFPS			= 0.0;
		double						mAverageDecayFPS	= 0.9;
		double						mAccumFrametime		= 1.0;

		uInt64						mCurrentFrameIdx	= 0;

		uSize						mMaxInFlightCount;
		VulkanCommandBuffer*		mCommandBuffers[VULKAN_GRAPHICS_MAX_IN_FLIGHT];
		VulkanImage*				mColorImages[VULKAN_GRAPHICS_MAX_IN_FLIGHT];
		VulkanImageView*			mColorImageViews[VULKAN_GRAPHICS_MAX_IN_FLIGHT];
		VulkanImage*				mDepthImages[VULKAN_GRAPHICS_MAX_IN_FLIGHT];
		VulkanImageView*			mDepthImageViews[VULKAN_GRAPHICS_MAX_IN_FLIGHT];

		Entity						mCameraEntity;

		VulkanSceneRenderer			mSceneRenderer;
		VulkanTerrainRenderer		mTerrainRenderer;
		VulkanSkyRenderer			mSkyRenderer;
		VulkanImGuiRenderer			mImGuiRenderer;

		VkSampler					mVkTonemapSampler;
		VulkanGraphicsPipeline*		mpTonemapPipeline;

	public:
		VulkanRenderer(VulkanGraphics& graphics, VulkanDevice& device, Window& activeWindow, uSize maxInFlightCount);

		void OnInitialize() override;
		void OnDestroy() override;

		void OnUpdate(double deltaTime) override;
		void OnBuildFrame(FrameGraph& frameGraph) override;

		void OnBackbufferChanged(uSize count, FrameGraphImageInfo& imageInfo) override;

		void SetCamera(Entity cameraEntity);

		void RecordTransfers(VulkanCommandRecorder& recorder, uInt32 frameIdx);
		void RecordPreDraws(VulkanCommandRecorder& recorder, uInt32 frameIdx);
		void RecordDraws(VulkanCommandRecorder& recorder, uInt32 frameIdx);
		void RecordPostDraws(VulkanCommandRecorder& recorder, uInt32 frameIdx);

		void RecordTonemapDraws(VulkanCommandRecorder& recorder, uInt32 frameIdx);

		void RenderScene(EntityWorld& world, uSize frameIdx);

		void SetTargetFPS(uInt64 fps);

		uInt64 GetTargetFPS() const { return mTargetFPS; }
		double GetCurrentFPS() const { return mCurrentFPS; }
		double GetAverageFPS() const { return mAverageFPS; }

		Window&	GetWindow() const { return *mpWindow; }
		uSize	GetMaxInFlightCount() const { return mMaxInFlightCount; }
	};
}