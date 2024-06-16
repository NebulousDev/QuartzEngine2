#pragma once

#include "../GfxAPI.h"
#include "VulkanResourceManager.h"
#include "Primatives/VulkanPipeline.h"
#include "Graphics/Component/MaterialComponent.h"

namespace Quartz
{
	class QUARTZ_GRAPHICS_API VulkanPipelineCache
	{
	private:
		VulkanDevice*						mpDevice;
		VulkanResourceManager*				mpResourceManager;
		Array<VulkanGraphicsPipeline*>		mPipelines;
		Array<VulkanGraphicsPipelineInfo>	mPipelineInfos;

	public:
		VulkanPipelineCache();
		VulkanPipelineCache(VulkanDevice* pDevice, VulkanResourceManager* pResources);

		VulkanGraphicsPipelineInfo MakeGraphicsPipelineInfo(
			const Array<VulkanShader*, 8>& shaders,
			const Array<VulkanAttachment>& attachments);

		VulkanGraphicsPipelineInfo MakeGraphicsPipelineInfo(
			const Array<VulkanShader*, 8>& shaders,
			const Array<VulkanAttachment>& attachments,
			const Array<VkVertexInputAttributeDescription>& vertexAttributes,
			const Array<VkVertexInputBindingDescription>& vertexBindings);

		VulkanGraphicsPipeline* FindOrCreateGraphicsPipeline(
			const VulkanGraphicsPipelineInfo& pipelineInfo);

	};
}