#pragma once

#include "Types/String.h"
#include "Types/Array.h"

#include <vulkan/vulkan.h>

namespace Quartz
{
	struct VulkanDevice;

	enum VulkanAttachmentType
	{
		VULKAN_ATTACHMENT_TYPE_SWAPCHAIN,
		VULKAN_ATTACHMENT_TYPE_COLOR,
		VULKAN_ATTACHMENT_TYPE_DEPTH,
		VULKAN_ATTACHMENT_TYPE_STENCIL,
		VULKAN_ATTACHMENT_TYPE_DEPTH_STENCIL
	};

	struct VulkanAttachment
	{
		String					name;
		VulkanAttachmentType	type;
		VkFormat				vkFormat;
	};

	struct VulkanSubpass
	{
		String					name;
		Array<uInt32>			attachments;
	};

	struct VulkanRenderpassInfo
	{
		String					name;
		Array<VulkanAttachment>	attachments;
		Array<VulkanSubpass>	subpasses;
	};

	struct VulkanRenderpass
	{
		VkRenderPass			vkRenderpass;
		VulkanDevice*			pDevice;
		Array<VulkanAttachment>	attachments;
		Array<VulkanSubpass>	subpasses;
		VkRenderPassCreateInfo	vkInfo;
	};
}