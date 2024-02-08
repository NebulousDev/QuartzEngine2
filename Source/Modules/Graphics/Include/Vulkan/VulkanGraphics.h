#pragma once

#include "GfxDLL.h"
#include "Graphics.h"
#include "VulkanDevice.h"
#include "VulkanPhysicalDevice.h"
#include "VulkanState.h"
#include "VulkanSwapchain.h"
#include "VulkanResourceManager.h"

namespace Quartz
{
	struct VulkanSubmission
	{
		Array<VulkanCommandBuffer*> commandBuffers;
		Array<VkSemaphore>			waitSemaphores;
		Array<VkPipelineStageFlags>	waitStages;
		Array<VkSemaphore>			signalSemaphores;
	};

	struct QUARTZ_GRAPHICS_API VulkanGraphics : public GraphicsInstance
	{
		VkInstance					vkInstance;
		VkApplicationInfo			vkAppInfo;
		Array<const char*>			enabledExtensions;
		VulkanPhysicalDevice		primaryPhysicalDevice;
		Array<VulkanPhysicalDevice> physicalDevices;
		VulkanDevice*				pPrimaryDevice;
		Array<VulkanDevice>			devices;
		VulkanState*				pState;
		VulkanResourceManager*		pResourceManager;

		//TEMP
		VulkanSurface* pSurface;

		bool TestVersion(uInt32 version);

		bool Create();
		void Destroy();

		void Submit(VulkanSubmission submission, VkQueue deviceQueue, VkFence signalFence);
	};

	
}