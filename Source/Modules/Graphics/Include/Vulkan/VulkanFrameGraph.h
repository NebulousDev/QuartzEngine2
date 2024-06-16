#pragma once

#include "Graphics/FrameGraph/FrameGraph.h"
#include "Vulkan/VulkanSwapchainTimer.h"
#include "Vulkan/VulkanGraphics.h"

namespace Quartz
{
	class VulkanFrameGraph : public FrameGraph
	{
	private:
		VulkanGraphics*									mpGraphics;
		VulkanResourceManager*							mpResourceManager;
		VulkanDevice*									mpDevice;

		VulkanCommandPool*								mpGraphicsPool;
		VulkanCommandPool*								mpComputePool;
		VulkanCommandPool*								mpTransferPool;

		Map<uInt64, Array<VulkanImage*, 3>, 128>		mImages;
		Map<uInt64, Array<VkFence, 3>, 128>				mImageFences;
		Map<uInt64, Array<VulkanImageView*, 3>, 128>	mImageViews;
		Map<uInt64, Array<VulkanBuffer*, 3>, 128>		mBuffers;
		Map<uInt64, VulkanSurface*, 8>					mOutputSurfaces;
		Map<uInt64, VulkanSwapchainTimer, 8>			mOutputSwapchains;
		Array<VulkanCommandBuffer*, 3>					mCommandBuffers;
		Array<VulkanCommandRecorder, 3>					mRecorders;
		uSize											mBackbufferCount;
		uSize											mBackbufferIdx;

	private:
		VulkanImage*		GetActiveImage(FrameGraphImage& graphImage);
		VulkanBuffer*		GetActiveBuffer(FrameGraphBuffer& graphBuffer);

		void				TransitionImage(
								VulkanCommandRecorder& commandRecorder,
								const FrameGraphImageTransition& oldState,
								const FrameGraphImageTransition& newState);

		void				PassTransitionImages(VulkanCommandRecorder& recorder, const FrameGraphPass& pass);

	private:
		bool				ApiInitialize(GraphicsApiInfo& info) override;
		bool				ApiDestroy() override;
		ApiImageHandle		ApiAquireImage(const FrameGraphImage& graphImage) override;
		void				ApiReleaseImage(const FrameGraphImage& graphImage) override;
		ApiBufferHandle		ApiAquireBuffer(const FrameGraphBuffer& graphBuffer) override;
		void				ApiReleaseBuffer(const FrameGraphBuffer& graphBuffer) override;
		ApiRecorderHandle	ApiAquireRecorder(const FrameGraphCommandRecorder& graphRecorder) override;
		void				ApiReleaseRecorder(const FrameGraphCommandRecorder& graphRecorder) override;
		void				ApiExecuteFrame() override;

	public:
		VulkanFrameGraph();

		VulkanImageView* GetActiveImageView(const WrapperString& imageName);

		// @TODO: Temp, use command recorder passed to Execute()
		VulkanCommandRecorder& GetActiveRecorder();
	};
}