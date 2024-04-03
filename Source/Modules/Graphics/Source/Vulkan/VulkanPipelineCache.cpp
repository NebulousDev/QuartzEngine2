#include "Vulkan/VulkanPipelineCache.h"

namespace Quartz
{
    VulkanGraphicsPipelineInfo VulkanPipelineCache::MakeGraphicsPipelineInfo(
		const Array<VulkanShader*>& shaders,
		const Array<VulkanAttachment>& attachments,
		const Array<VkVertexInputAttributeDescription>& vertexAttributes,
		const Array<VkVertexInputBindingDescription>& vertexBindings)
    {
        VulkanGraphicsPipelineInfo pipelineInfo;

		// Intentional memset to ensure memcmp works with padding later
		memset(&pipelineInfo, 0, sizeof(VulkanGraphicsPipelineInfo));

		pipelineInfo.shaders				= shaders;
		pipelineInfo.attachments			= attachments;
		pipelineInfo.vkTopology				= VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		pipelineInfo.vkPolygonMode			= VK_POLYGON_MODE_FILL;
		pipelineInfo.vkCullMode				= VK_CULL_MODE_BACK_BIT;
		pipelineInfo.vkFrontFace			= VK_FRONT_FACE_COUNTER_CLOCKWISE;
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

	// NOTE: This will use the SPIRV format guess and may not be correct
	//       Also assumes one input buffer with vertex input rate at binding=0
	VulkanGraphicsPipelineInfo VulkanPipelineCache::MakeGraphicsPipelineInfo(
		const Array<VulkanShader*>& shaders,
		const Array<VulkanAttachment>& attachments)
	{
		Array<VkVertexInputAttributeDescription> vertexAttributes;
		Array<VkVertexInputBindingDescription> vertexBindings;

		uSize totalSize = 0;

		// @TODO: make a metadata file for these properties
		for (VulkanShader* pShader : shaders)
		{
			if (!(pShader->vkStage & VK_SHADER_STAGE_VERTEX_BIT))
			{
				continue; // Look for the vertex shader only
			}

			for (SpirvAttribute attrib : pShader->attributes)
			{
				VkVertexInputAttributeDescription vkAttributeDesc = {};
				vkAttributeDesc.binding		= attrib.binding;
				vkAttributeDesc.format		= attrib.formatGuess;
				vkAttributeDesc.location	= attrib.location;
				vkAttributeDesc.offset		= totalSize;

				vertexAttributes.PushBack(vkAttributeDesc);
				totalSize += attrib.size;
			}
		}

		VkVertexInputBindingDescription vertexBufferAttachment = {};
		vertexBufferAttachment.binding		= 0;
		vertexBufferAttachment.stride		= totalSize;
		vertexBufferAttachment.inputRate	= VK_VERTEX_INPUT_RATE_VERTEX;

		vertexBindings.PushBack(vertexBufferAttachment);

		return MakeGraphicsPipelineInfo(shaders, attachments, vertexAttributes, vertexBindings);
	}

	VulkanPipelineCache::VulkanPipelineCache() :
		mpDevice(nullptr), mpResourceManager(nullptr)
	{
		// Nothing
	}

	VulkanPipelineCache::VulkanPipelineCache(VulkanDevice* pDevice, VulkanResourceManager* pResources) :
        mpDevice(pDevice), mpResourceManager(pResources)
    {
        // Nothing
    }

    VulkanGraphicsPipeline* VulkanPipelineCache::FindOrCreateGraphicsPipeline(
		const VulkanGraphicsPipelineInfo& pipelineInfo)
    {   
		VulkanGraphicsPipelineInfo*		pFoundPipelineInfo = nullptr;
		uSize							pipelineIndex = -1;

		for (VulkanGraphicsPipelineInfo& info : mPipelineInfos)
		{
			pipelineIndex++;

			if (pipelineInfo.shaders.Size() != info.shaders.Size())
				continue;

			for (uSize i = 0; i < pipelineInfo.shaders.Size(); i++)
			{
				if (pipelineInfo.shaders[i]->vkShader != info.shaders[i]->vkShader)
					goto breakContinue;
			}

			if (pipelineInfo.attachments.Size() != info.attachments.Size())
				continue;

			for (uSize i = 0; i < pipelineInfo.attachments.Size(); i++)
			{
				if (pipelineInfo.attachments[i].vkFormat != info.attachments[i].vkFormat)
					goto breakContinue;

				if (pipelineInfo.attachments[i].type != info.attachments[i].type)
					goto breakContinue;
			}

			if (pipelineInfo.vertexAttributes.Size() != info.vertexAttributes.Size())
				continue;

			for (uSize i = 0; i < pipelineInfo.vertexAttributes.Size(); i++)
			{
				uSize result = memcmp(&pipelineInfo.vertexAttributes[i], &info.vertexAttributes[i], sizeof(VkVertexInputAttributeDescription));

				if (result != 0)
					continue;
			}

			if (pipelineInfo.vertexBindings.Size() != info.vertexBindings.Size())
				continue;

			for (uSize i = 0; i < pipelineInfo.vertexBindings.Size(); i++)
			{
				uSize result = memcmp(&pipelineInfo.vertexBindings[i], &info.vertexBindings[i], sizeof(VkVertexInputBindingDescription));

				if (result != 0)
					goto breakContinue;
			}

			uSize offsetStart	= offsetof(VulkanGraphicsPipelineInfo, vkTopology);
			uSize offsetEnd		= offsetof(VulkanGraphicsPipelineInfo, useDynamicRendering) + 1; // +1 bool
			uSize size			= offsetEnd - offsetStart;

			uSize result = memcmp(((uInt8*)&info) + offsetStart, ((uInt8*)&pipelineInfo) + offsetStart, size);

			if (result != 0)
				continue;

			pFoundPipelineInfo = &info;

			break;

		breakContinue:
			continue;
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

