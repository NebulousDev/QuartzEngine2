#pragma once

#include "VulkanDevice.h"
#include "VulkanImage.h"
#include "VulkanRenderpass.h"
#include "Types/Map.h"

#include <vulkan/vulkan.h>

namespace Quartz
{
	struct VulkanShader;

	struct VulkanDescriptorSetInfo
	{
		uInt32								set;
		VkDescriptorSetLayout				vkDescriptorSetLayout;
		Array<VkDescriptorSetLayoutBinding> bindings;
		uInt32								sizeBytes;
	};

	struct VulkanDescriptorWriter
	{
		Array<VkWriteDescriptorSet>		descWrites;
		Array<VkDescriptorBufferInfo>	bufferInfos;
		Array<VkDescriptorImageInfo>	imageInfos;
		Map<uInt32, uInt32>				bindingTable;

		bool operator==(const VulkanDescriptorWriter& writer);

		void SetupWriter(const VulkanDescriptorSetInfo& info);
		void SetDynamicBuffer(uInt32 binding, VkBuffer vkBuffer, uInt32 offset, uInt32 range);
		void SetImageSampler(uInt32 binding, VkImageView vkImageView, VkSampler vkSampler);

		void UpdateDescriptorSet(VulkanDevice* pDevice, VkDescriptorSet vkDescriptorSet);
	};

	struct VulkanGraphicsPipelineInfo
	{
		Array<VulkanShader*>						shaders;
		Array<VkVertexInputBindingDescription>		bufferAttachments;
		Array<VkVertexInputAttributeDescription>	vertexAttributes;
		Array<VkPipelineColorBlendAttachmentState>	blendAttachments;
		bool										dynamicViewport;
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

		VulkanRenderpass*							pRenderpass;
	};

	struct VulkanGraphicsPipeline
	{
		typedef uInt32 SetID;

		VkPipeline							vkPipeline;
		VulkanDevice*						pDevice;
		VkGraphicsPipelineCreateInfo		vkPipelineInfo;
		Array<VulkanDescriptorSetInfo>		descriptorSetInfos;
		//Array<VulkanDescriptorCache>		descriptorCaches;
		//Map<SetID, VulkanDescriptorWriter>	descriptorWriters;
		VkSampler							defaultVkSampler;

		//void SetupUniformStates(uInt32 count);

		//VulkanDescriptorWriter* GetDescriptorWriter(uInt32 set);
		//VkDescriptorSet GetCashedDescriptorSet(uInt32 set, VulkanDescriptorWriter& writer, uInt32 frameIndex);
	};

	struct VulkanComputePipeline
	{
		// TODO
		VkPipeline vkPipeline;
	};
}