#include "Vulkan/VulkanSwapchainTimer.h"

namespace Quartz
{
	VulkanSwapchainTimer::VulkanSwapchainTimer() :
		mpSwapchain(nullptr),
		mInFlightFences(0),
		mImageIndex(0),
		mCurrentFrame(0)
	{
		// Nothing
	}

	VulkanSwapchainTimer::VulkanSwapchainTimer(VulkanSwapchain* pSwapchain) :
		mpSwapchain(pSwapchain),
		mInFlightFences(pSwapchain->imageFences.Size()),
		mImageIndex(0),
		mCurrentFrame(pSwapchain->backbufferCount - 1)
	{ }

	#define FENCE_TIMEOUT UINT64_MAX

	void VulkanSwapchainTimer::AdvanceFrame()
	{
		// @NOTE: doesn't have to be backbuffer count?
		mCurrentFrame = (mCurrentFrame + 1) % mpSwapchain->backbufferCount;

		vkWaitForFences(mpSwapchain->pDevice->vkDevice, 1, &mpSwapchain->imageFences[mCurrentFrame], VK_TRUE, FENCE_TIMEOUT);
		
		vkResetFences(mpSwapchain->pDevice->vkDevice, 1, &mpSwapchain->imageFences[mCurrentFrame]);
		
		vkAcquireNextImageKHR(mpSwapchain->pDevice->vkDevice, mpSwapchain->vkSwapchain,
			UINT64_MAX, mpSwapchain->imageAvailableSemaphores[mCurrentFrame], VK_NULL_HANDLE, &mImageIndex);
	}

	void VulkanSwapchainTimer::Present()
	{
		VkSemaphore		signalSemaphores[]	= { mpSwapchain->imageCompleteSemaphores[mCurrentFrame] };
		VkSwapchainKHR	swapChains[]		= { mpSwapchain->vkSwapchain };
		uInt32			imageIndices[]		= { mImageIndex };

		VkPresentInfoKHR presentInfo{};
		presentInfo.sType				= VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		presentInfo.waitSemaphoreCount	= 1;
		presentInfo.pWaitSemaphores		= signalSemaphores;
		presentInfo.swapchainCount		= 1;
		presentInfo.pSwapchains			= swapChains;
		presentInfo.pImageIndices		= imageIndices;
		presentInfo.pResults			= nullptr;

		vkQueuePresentKHR(mpSwapchain->pDevice->queues.present, &presentInfo);
	}

	VulkanImageView* VulkanSwapchainTimer::GetCurrentImageView()
	{
		return mpSwapchain->imageViews[mCurrentFrame];
	}

	VkSemaphore VulkanSwapchainTimer::GetCurrentAcquiredSemaphore()
	{
		return mpSwapchain->imageAvailableSemaphores[mCurrentFrame];
	}

	VkSemaphore VulkanSwapchainTimer::GetCurrentCompleteSemaphore()
	{
		return mpSwapchain->imageCompleteSemaphores[mCurrentFrame];
	}

	VkFence VulkanSwapchainTimer::GetCurrentFence()
	{
		return mpSwapchain->imageFences[mCurrentFrame];
	}
}

