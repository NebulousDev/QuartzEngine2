#include "Vulkan/VulkanCommandRecorder.h"

#include "Log.h"

namespace Quartz
{
	VulkanCommandRecorder::VulkanCommandRecorder(VulkanCommandBuffer* pCommandBuffer) :
		mpCommandBuffer(pCommandBuffer)
	{
		// Nothing
	}

	void VulkanCommandRecorder::BeginRecording()
	{
		VkCommandBufferBeginInfo beginInfo = {};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

		if (vkBeginCommandBuffer(mpCommandBuffer->vkCommandBuffer, &beginInfo) != VK_SUCCESS)
		{
			LogError("Failed to begin command buffer recording: vkBeginCommandBuffer failed!");
			return;
		}
	}

	void VulkanCommandRecorder::EndRecording()
	{
		if (vkEndCommandBuffer(mpCommandBuffer->vkCommandBuffer) != VK_SUCCESS)
		{
			LogError("Failed to end command buffer recording: vkBeginCommandBuffer failed!");
		}
	}

	void VulkanCommandRecorder::BeginRenderpass(VulkanRenderpass* pRenderpass, const VulkanRenderpassBeginInfo& beginInfo)
	{
		VkRenderPassBeginInfo renderPassInfo = {};
		renderPassInfo.sType				= VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass			= pRenderpass->vkRenderpass;
		renderPassInfo.framebuffer			= beginInfo.pFramebuffer->vkFramebuffer;
		renderPassInfo.renderArea			= beginInfo.renderArea;
		renderPassInfo.clearValueCount		= beginInfo.clearValues.Size();
		renderPassInfo.pClearValues			= beginInfo.clearValues.Data();

		vkCmdBeginRenderPass(mpCommandBuffer->vkCommandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE); // Only inline for now
	}

	void VulkanCommandRecorder::EndRenderpass()
	{
		vkCmdEndRenderPass(mpCommandBuffer->vkCommandBuffer);
	}

	void VulkanCommandRecorder::SetGraphicsPipeline(VulkanGraphicsPipeline* pPipeline)
	{
		vkCmdBindPipeline(mpCommandBuffer->vkCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pPipeline->vkPipeline);
	}

	void VulkanCommandRecorder::SetComputePipeline(VulkanComputePipeline* pPipeline)
	{
		vkCmdBindPipeline(mpCommandBuffer->vkCommandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, pPipeline->vkPipeline);
	}

	void VulkanCommandRecorder::SetVertexBuffers(const Array<VulkanBufferBind>& buffers)
	{
		constexpr const uSize bufferCacheSize = 16;

		VkBuffer		vkBuffers[bufferCacheSize] = {};
		VkDeviceSize	vkOffsetSizes[bufferCacheSize] = {};

		if (buffers.Size() > bufferCacheSize) // @TODO: should be debug_assert
		{
			LogError("SetVertexBuffers failed. buffers.Size() > bufferCacheSize");
		}

		for (uSize i = 0; i < buffers.Size(); i++)
		{
			vkBuffers[i]		= buffers[i].pBuffer->vkBuffer;
			vkOffsetSizes[i]	= buffers[i].offset;
		}

		vkCmdBindVertexBuffers(mpCommandBuffer->vkCommandBuffer, 0, buffers.Size(), vkBuffers, vkOffsetSizes);
	}

	void VulkanCommandRecorder::SetIndexBuffer(VulkanBuffer* pIndexBuffer, uSize offset, VkIndexType indexType)
	{
		vkCmdBindIndexBuffer(mpCommandBuffer->vkCommandBuffer, pIndexBuffer->vkBuffer, offset, indexType);
	}

	void VulkanCommandRecorder::BindUniforms(VulkanGraphicsPipeline* pPipeline, uInt32 set, const Array<VulkanUniformBinding>& bindings)
	{
		constexpr const uSize maxWriteDescriptorSets = 16;

		VkWriteDescriptorSet writeDescriptorSets[maxWriteDescriptorSets] = {};

		for (uSize i = 0; i < bindings.Size(); i++)
		{
			VkDescriptorBufferInfo bufferInfo = {};
			bufferInfo.buffer	= bindings[i].pBuffer->vkBuffer;
			bufferInfo.offset	= bindings[i].offset;
			bufferInfo.range	= bindings[i].range;

			writeDescriptorSets[i].sType			= VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			writeDescriptorSets[i].dstSet			= 0; // Ignored
			writeDescriptorSets[i].dstBinding		= bindings[i].binding;
			writeDescriptorSets[i].descriptorCount	= 1;
			writeDescriptorSets[i].descriptorType	= VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			writeDescriptorSets[i].pBufferInfo		= &bufferInfo;
		}

		PFN_vkCmdPushDescriptorSetKHR vkCmdPushDescriptorSetKHR2 = 
			(PFN_vkCmdPushDescriptorSetKHR)vkGetDeviceProcAddr(pPipeline->pDevice->vkDevice, "vkCmdPushDescriptorSetKHR");

		vkCmdPushDescriptorSetKHR2(mpCommandBuffer->vkCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, 
			pPipeline->vkPipelineInfo.layout, set, bindings.Size(), writeDescriptorSets);
	}

	void VulkanCommandRecorder::DrawIndexed(uInt32 instanceCount, uInt32 indexCount, uInt32 indexStart)
	{
		vkCmdDrawIndexed(mpCommandBuffer->vkCommandBuffer, indexCount, instanceCount, indexStart, 0, 0);
	}

	void VulkanCommandRecorder::CopyBuffer(VulkanBuffer* pSrcBuffer, VulkanBuffer* pDestBuffer, uSize sizeBytes, uSize srcOffset, uSize destOffset)
	{
		VkBufferCopy copyRegion = {};
		copyRegion.srcOffset	= srcOffset;
		copyRegion.dstOffset	= destOffset;
		copyRegion.size			= sizeBytes;

		vkCmdCopyBuffer(mpCommandBuffer->vkCommandBuffer, pSrcBuffer->vkBuffer, pDestBuffer->vkBuffer, 1, &copyRegion);
	}
}