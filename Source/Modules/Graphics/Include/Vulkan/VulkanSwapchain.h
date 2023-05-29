#pragma once

#include "Vulkan/VulkanImage.h"
#include "Vulkan/VulkanDevice.h"

#include "Types/Array.h"

namespace Quartz
{
	class VulkanSwapchain
	{
	private:
		VkSwapchainKHR			mvkSwapchain;
		VulkanDevice*			mpDevice;
		uInt32					mBackbufferCount;

		Array<VulkanImage*>		mImages;
		Array<VulkanImageView*>	mImageViews;
		Array<VkSemaphore>		mImageAvailableSemaphores;
		Array<VkSemaphore>		mImageCompleteSemaphores;
		Array<VkFence>			mImageFences;
		Array<VkFence>			mInFlightFences;

		uInt32					mImageIndex;
		uInt32					mCurrentFrame;

	public:
		VulkanSwapchain() {};
		VulkanSwapchain(VkSwapchainKHR vkSwapchain,
			VulkanDevice* pDevice,
			uInt32 backbufferCount,
			const Array<VulkanImage*>& images,
			const Array<VulkanImageView*>& imageViews,
			const Array<VkSemaphore>& imageAcquiredSemaphores,
			const Array<VkSemaphore>& imageCompleteSemaphores,
			const Array<VkFence>& imageFences,
			const Array<VkFence>& inFlightFences);

		void Rebuild();

		void AdvanceFrame();
		void Present();

		VulkanImageView*	GetCurrentImageView();
		VkSemaphore			GetCurrentAcquiredSemaphore();
		VkSemaphore			GetCurrentCompleteSemaphore();
		VkFence				GetCurrentFence();

		uInt32				GetFrameIndex() { return mImageIndex; }
		uInt32				GetCurrentFrame() { return mCurrentFrame; }

		uInt32							GetBackbufferCount() { return mBackbufferCount; }
		VkSwapchainKHR					GetVkSwapchain() { return mvkSwapchain; }
		const Array<VulkanImage*>&		GetImages() const { return mImages; }
		const Array<VulkanImageView*>&	GetImageViews() const { return mImageViews; }
	};
}