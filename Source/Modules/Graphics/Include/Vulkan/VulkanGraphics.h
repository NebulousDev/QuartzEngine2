#pragma once

#include "GfxAPI.h"
#include "Graphics.h"
#include "Primatives/VulkanDevice.h"
#include "Primatives/VulkanPhysicalDevice.h"
#include "Primatives/VulkanSwapchain.h"
#include "VulkanResourceManager.h"
#include "VulkanShaderCache.h"
#include "VulkanBufferCache.h"
#include "VulkanPipelineCache.h"

#include "Entity/World.h"

#define VULKAN_GRAPHICS_MAX_IN_FLIGHT 8

namespace Quartz
{
	struct VulkanSubmission
	{
		Array<VulkanCommandBuffer*> commandBuffers;
		Array<VkSemaphore>			waitSemaphores;
		Array<VkPipelineStageFlags>	waitStages;
		Array<VkSemaphore>			signalSemaphores;
	};

	struct QUARTZ_GRAPHICS_API VulkanGraphics
	{
		VkInstance					vkInstance;
		VkApplicationInfo			vkAppInfo;
		Array<const char*>			enabledExtensions;
		VulkanPhysicalDevice		primaryPhysicalDevice;
		Array<VulkanPhysicalDevice> physicalDevices;
		VulkanDevice*				pPrimaryDevice;
		Array<VulkanDevice>			devices;
		VulkanResourceManager		resourceManager;
		VulkanShaderCache			shaderCache;
		VulkanBufferCache			bufferCache;
		VulkanPipelineCache			pipelineCache;

		//TEMP
		VulkanSurface* pSurface;

		bool TestVersion(uInt32 version);

		bool Create();
		void Destroy();

		void WaitIdle();
		void Submit(VulkanSubmission submission, VkQueue deviceQueue, VkFence signalFence);
	};

	
}