#pragma once

#include "Graphics.h"
#include "VulkanDevice.h"
#include "VulkanPhysicalDevice.h"
#include "VulkanState.h"
#include "VulkanSwapchain.h"
#include "VulkanResourceManager.h"

namespace Quartz
{
	struct VulkanGraphics : public GraphicsInstance
	{
		VkInstance					vkInstance;
		VkApplicationInfo			vkAppInfo;
		Array<const char*>			enabledExtensions;
		VulkanPhysicalDevice		primaryPhysicalDevice;
		Array<VulkanPhysicalDevice> physicalDevices;
		VulkanDevice				primaryDevice;
		Array<VulkanDevice>			devices;
		VulkanState*				pState;
		VulkanResourceManager*		pResourceManager;

		//TEMP
		VulkanSurface* pSurface;
	};

	bool TestVulkan(uInt32 version);

	bool CreateVulkan(VulkanGraphics* pGraphics);
	void DestroyVulkan(VulkanGraphics* pGraphics);


}