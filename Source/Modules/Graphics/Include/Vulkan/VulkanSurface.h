#pragma once

#include "Types/Array.h"

#include <vulkan/vulkan.h>

namespace Quartz
{
	class VulkanSurface
	{
	private:
		VkSurfaceKHR				mvkSurface;
		Array<VkSurfaceFormatKHR>	mSupportedFormats;
		VkSurfaceCapabilitiesKHR	mCapibilites;

		uInt32						mWidth;
		uInt32						mHeight;

	public:
		VulkanSurface() {};
		VulkanSurface(VkSurfaceKHR vkSurface, uInt32 width, uInt32 height,
			const Array<VkSurfaceFormatKHR>& formats, VkSurfaceCapabilitiesKHR capibilites);

		VkSurfaceKHR						GetVkSurface() { return mvkSurface; };
		const Array<VkSurfaceFormatKHR>&	GetSupportedFormats() const { return mSupportedFormats; }
		VkSurfaceCapabilitiesKHR			GetCapibilites() { return mCapibilites; }
	};
}