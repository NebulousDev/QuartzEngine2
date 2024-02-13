#include "Vulkan/VulkanPipelineManager.h"

namespace Quartz
{
    VulkanGraphicsPipelineInfo VulkanPipelineManager::MakeGraphicsPipelineInfo(
		const Array<VulkanShader*>& shaders,
		const Array<VulkanAttachment>& attachments,
		const Array<VkVertexInputAttributeDescription>& vertexAttributes,
		const Array<VkVertexInputBindingDescription>& vertexBindings)
    {
        VulkanGraphicsPipelineInfo pipelineInfo = {};
		pipelineInfo.shaders				= shaders;
		pipelineInfo.attachments			= attachments;
		pipelineInfo.vkTopology				= VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		pipelineInfo.vkPolygonMode			= VK_POLYGON_MODE_FILL;
		pipelineInfo.vkCullMode				= VK_CULL_MODE_BACK_BIT;
		pipelineInfo.vkFrontFace			= VK_FRONT_FACE_CLOCKWISE;
		pipelineInfo.lineWidth				= 1.0f;
		pipelineInfo.multisamples			= VK_SAMPLE_COUNT_1_BIT;
		pipelineInfo.depth.enableTesting	= true;
		pipelineInfo.depth.enableWrite		= true;
		pipelineInfo.depth.compareOp		= VK_COMPARE_OP_LESS_OR_EQUAL;
		pipelineInfo.depth.depthMin			= 0.0f;
		pipelineInfo.depth.depthMax			= 1.0f;
		pipelineInfo.stencil.enableTesting	= false;
		pipelineInfo.stencil.compareOp		= VK_COMPARE_OP_LESS_OR_EQUAL;
		pipelineInfo.usePushDescriptors		= true;
		pipelineInfo.useDynamicViewport		= true;
		pipelineInfo.useDynamicRendering	= true;
		pipelineInfo.vertexAttributes		= vertexAttributes;
		pipelineInfo.vertexBindings			= vertexBindings;
		pipelineInfo.pRenderpass			= nullptr; // Dynamic Rendering

		VkPipelineColorBlendAttachmentState	blendAttachment = {};
		blendAttachment.blendEnable			= VK_TRUE;
		blendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
		blendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
		blendAttachment.colorBlendOp		= VK_BLEND_OP_ADD;
		blendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
		blendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
		blendAttachment.alphaBlendOp		= VK_BLEND_OP_ADD;
		blendAttachment.colorWriteMask		= VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;

		pipelineInfo.blendAttachments.PushBack(blendAttachment);

        return pipelineInfo;
    }

	VulkanPipelineManager::VulkanPipelineManager() :
		mpDevice(nullptr), mpResourceManager(nullptr)
	{
		// Nothing
	}

	VulkanPipelineManager::VulkanPipelineManager(VulkanDevice* pDevice, VulkanResourceManager* pResources) :
        mpDevice(pDevice), mpResourceManager(pResources)
    {
        // Nothing
    }

    VulkanGraphicsPipeline* VulkanPipelineManager::FindOrCreateGraphicsPipeline(
		const Array<VulkanShader*>& shaders,
		const Array<VulkanAttachment>& attachments,
		const Array<VkVertexInputAttributeDescription>& vertexAttributes,
		const Array<VkVertexInputBindingDescription>& vertexBindings)
    {   
		VulkanGraphicsPipelineInfo		pipelineInfo = MakeGraphicsPipelineInfo(shaders, attachments, vertexAttributes, vertexBindings);
		VulkanGraphicsPipelineInfo*		pFoundPipelineInfo = nullptr;
		uSize							pipelineIndex = -1;

		for (VulkanGraphicsPipelineInfo& info : mPipelineInfos)
		{
			pipelineIndex++;

			if (shaders.Size() != info.shaders.Size())
				continue;

			for (uSize i = 0; i < shaders.Size(); i++)
			{
				if (shaders[i]->vkShader != info.shaders[i]->vkShader)
					continue;
			}

			if (attachments.Size() != info.attachments.Size())
				continue;

			for (uSize i = 0; i < attachments.Size(); i++)
			{
				if (attachments[i].vkFormat != info.attachments[i].vkFormat)
					continue;

				if (attachments[i].type != info.attachments[i].type)
					continue;
			}

			if (vertexAttributes.Size() != info.vertexAttributes.Size())
				continue;

			for (uSize i = 0; i < vertexAttributes.Size(); i++)
			{
				uSize result = memcmp(&vertexAttributes[i], &info.vertexAttributes[i], sizeof(VkVertexInputAttributeDescription));

				if (result != 0)
					continue;
			}

			if (vertexBindings.Size() != info.vertexBindings.Size())
				continue;

			for (uSize i = 0; i < vertexBindings.Size(); i++)
			{
				uSize result = memcmp(&vertexBindings[i], &info.vertexBindings[i], sizeof(VkVertexInputBindingDescription));

				if (result != 0)
					continue;
			}

			uSize result = memcmp(
				&info + offsetof(VulkanGraphicsPipelineInfo, viewport),
				&pipelineInfo + offsetof(VulkanGraphicsPipelineInfo, viewport),
				sizeof(VulkanGraphicsPipelineInfo));

			if (result != 0)
				continue;

			pFoundPipelineInfo = &info;

			break;
		}

		if (pFoundPipelineInfo)
		{
			return mPipelines[pipelineIndex];
		}
		else
		{
			VulkanGraphicsPipeline* pGraphicsPipeline = mpResourceManager->CreateGraphicsPipeline(mpDevice, pipelineInfo, 0);
			mPipelines.PushBack(pGraphicsPipeline);
			mPipelineInfos.PushBack(pipelineInfo);

			return pGraphicsPipeline;
		}
    }
}

