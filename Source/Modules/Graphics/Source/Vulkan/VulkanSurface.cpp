#include "Vulkan/VulkanSurface.h"

namespace Quartz
{
	VulkanSurface::VulkanSurface(VkSurfaceKHR vkSurface, uInt32 width, uInt32 height,
		const Array<VkSurfaceFormatKHR>& formats, VkSurfaceCapabilitiesKHR capibilites) : 
		mvkSurface(vkSurface),
		mSupportedFormats(formats),
		mCapibilites(capibilites)
	{ }
}

