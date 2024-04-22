#pragma once

#include "../GfxAPI.h"
#include "Math/Math.h"
#include "Runtime/Runtime.h"

#include "VulkanGraphics.h"
#include "VulkanBufferCache.h"
#include "VulkanSwapchainTimer.h"
#include "VulkanBufferWriter.h"

#include "../SceneRenderer.h"
#include "../TerrainRenderer.h"
#include "../SkyRenderer.h"
#include "../ImGuiRenderer.h"

namespace Quartz
{
	class QUARTZ_GRAPHICS_API VulkanRenderer
	{
	private:
		VulkanGraphics*				mpGraphics;
		VulkanSwapchain*			mpSwapchain;
		VulkanSwapchainTimer		mSwapTimer;

		VulkanBufferCache			mBufferCache;
		VulkanShaderCache			mShaderCache;
		VulkanPipelineCache			mPipelineCache;

		uSize						mMaxInFlightCount;
		VulkanCommandBuffer*		mCommandBuffers[VULKAN_GRAPHICS_MAX_IN_FLIGHT];
		VulkanImage*				mDepthImages[VULKAN_GRAPHICS_MAX_IN_FLIGHT];
		VulkanImageView*			mDepthImageViews[VULKAN_GRAPHICS_MAX_IN_FLIGHT];

		Entity						mCameraEntity;

		VulkanSceneRenderer			mSceneRenderer;
		VulkanTerrainRenderer		mTerrainRenderer;
		VulkanSkyRenderer			mSkyRenderer;
		VulkanImGuiRenderer			mImGuiRenderer;

	public:
		void Initialize(VulkanGraphics& graphics, void* pWindowHandle, uSize maxInFlightCount);

		void SetCamera(Entity cameraEntity);

		void UpdateAll(EntityWorld& world, uSize frameIdx);
		void RecordTransfers(VulkanCommandRecorder& recorder, uInt32 frameIdx);
		void RecordPreDraws(VulkanCommandRecorder& recorder, uInt32 frameIdx);
		void RecordDraws(VulkanCommandRecorder& recorder, uInt32 frameIdx);
		void RecordPostDraws(VulkanCommandRecorder& recorder, uInt32 frameIdx);

		void RenderScene(EntityWorld& world, uSize frameIdx);

		void RenderUpdate(Runtime& runtime, double delta);
		void Register(Runtime& runtime);

		uSize GetMaxInFlightCount() const { return mMaxInFlightCount; }
		VulkanBufferCache& GetBufferCache() { return mBufferCache; }
		VulkanShaderCache& GetShaderCache() { return mShaderCache; }
		VulkanPipelineCache& GetPipelineCache() { return mPipelineCache; }
	};
}