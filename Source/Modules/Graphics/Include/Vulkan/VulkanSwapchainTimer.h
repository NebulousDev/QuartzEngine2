#pragma once

#include "GfxAPI.h"
#include "Primatives/VulkanSwapchain.h"

namespace Quartz
{
	class QUARTZ_GRAPHICS_API VulkanSwapchainTimer
	{
		private:
			VulkanSwapchain*		mpSwapchain;
			Array<VkFence>			mInFlightFences;
			uInt32					mImageIndex;
			uInt32					mCurrentFrame;

		public:
			VulkanSwapchainTimer(VulkanSwapchain* pSwapchain);

			void AdvanceFrame();
			void Present();

			VulkanImageView*	GetCurrentImageView();
			VkSemaphore			GetCurrentAcquiredSemaphore();
			VkSemaphore			GetCurrentCompleteSemaphore();
			VkFence				GetCurrentFence();

			uInt32				GetFrameIndex() { return mImageIndex; }
			uInt32				GetCurrentFrame() { return mCurrentFrame; }

			VulkanSwapchain*	GetSwapchain() { return mpSwapchain; }
	};
}