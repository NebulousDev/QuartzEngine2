#pragma once

#include "../GfxAPI.h"
#include "VulkanResourceManager.h"
#include "Primatives/VulkanPipeline.h"
#include "Component/MaterialComponent.h"

namespace Quartz
{
	class QUARTZ_GRAPHICS_API VulkanPipelineCache
	{
	private:
		VulkanDevice*						mpDevice;
		VulkanResourceManager*				mpResourceManager;
		Array<VulkanGraphicsPipeline*>		mPipelines;
		Array<VulkanGraphicsPipelineInfo>	mPipelineInfos;

		VulkanGraphicsPipelineInfo MakeGraphicsPipelineInfo(
			const Array<VulkanShader*>& shaders,
			const Array<VulkanAttachment>& attachments,
			const Array<VkVertexInputAttributeDescription>& vertexAttributes,
			const Array<VkVertexInputBindingDescription>& vertexBindings);

	public:
		VulkanPipelineCache();
		VulkanPipelineCache(VulkanDevice* pDevice, VulkanResourceManager* pResources);

		VulkanGraphicsPipeline* FindOrCreateGraphicsPipeline(
			const Array<VulkanShader*>& shaders,
			const Array<VulkanAttachment>& attachments,
			const Array<VkVertexInputAttributeDescription>& vertexAttributes,
			const Array<VkVertexInputBindingDescription>& vertexBindings);

	};
}