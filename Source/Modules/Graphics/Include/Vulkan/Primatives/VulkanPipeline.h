#pragma once

#include "VulkanDevice.h"
#include "VulkanImage.h"
#include "VulkanRenderpass.h"
#include "VulkanDescriptorSetLayout.h"
#include "Types/Map.h"

#include <vulkan/vulkan.h>

namespace Quartz
{
	struct VulkanShader;

	#pragma pack(push,1)
	struct VulkanGraphicsPipelineInfo
	{
		Array<VulkanShader*>						shaders;
		Array<VulkanAttachment>						attachments;
		Array<VkVertexInputBindingDescription>		vertexBindings;
		Array<VkVertexInputAttributeDescription>	vertexAttributes;
		Array<VkPipelineColorBlendAttachmentState>	blendAttachments;
		VulkanRenderpass*							pRenderpass;
		VkViewport									viewport;
		VkRect2D									scissor;
		VkPrimitiveTopology							vkTopology;
		VkPolygonMode								vkPolygonMode;
		VkCullModeFlags								vkCullMode;
		VkFrontFace									vkFrontFace;
		float										lineWidth;
		VkSampleCountFlagBits						multisamples;

		struct
		{
			bool									enableTesting;
			bool									enableWrite;
			VkCompareOp								compareOp;
			float									depthMin;
			float									depthMax;
		}
		depth;

		struct
		{
			bool									enableTesting;
			VkCompareOp								compareOp;
		}
		stencil;

		bool										usePushDescriptors;
		bool										useDynamicViewport;
		bool										useDynamicRendering;
	};
	#pragma pack(pop)

	struct VulkanGraphicsPipeline
	{
		VkPipeline							vkPipeline;
		VulkanDevice*						pDevice;
		VkGraphicsPipelineCreateInfo		vkPipelineInfo;
		VulkanGraphicsPipelineInfo			pipelineInfo;
		Array<VulkanDescriptorSetLayout*>	descriptorSetLayouts;
		VkSampler							defaultVkSampler;
	};

	struct VulkanComputePipeline
	{
		// TODO
		VkPipeline vkPipeline;
	};
}