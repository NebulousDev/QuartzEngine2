#pragma once

#include "VulkanRenderpass.h"
#include "VulkanImage.h"

namespace Quartz
{
	struct VulkanFramebufferInfo
	{
		VulkanRenderpass*		renderpass;
		Array<VulkanImageView*> attachments;
		uInt32					width;
		uInt32					height;
		uInt32					layers;
	};

	struct VulkanFramebuffer
	{
		VkFramebuffer			vkFramebuffer;
		VulkanRenderpass*		renderpass;
		Array<VulkanImageView*> attachments;
		uInt32					width;
		uInt32					height;
		uInt32					layers;
	};
}