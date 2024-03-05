#include "Vulkan/VulkanCommandRecorder.h"

#include "Log.h"

namespace Quartz
{
	VulkanCommandRecorder::VulkanCommandRecorder() :
		mpCommandBuffer(nullptr) { }

	VulkanCommandRecorder::VulkanCommandRecorder(VulkanCommandBuffer* pCommandBuffer) :
		mpCommandBuffer(pCommandBuffer) { }

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

	void VulkanCommandRecorder::Reset()
	{
		vkResetCommandBuffer(mpCommandBuffer->vkCommandBuffer, 0);
	}

	void VulkanCommandRecorder::BeginRenderpass(VulkanRenderpass* pRenderpass, const VulkanRenderpassBeginInfo& beginInfo)
	{
		VkRenderPassBeginInfo renderPassInfo = {};
		renderPassInfo.sType				= VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass			= pRenderpass->vkRenderpass;
		renderPassInfo.framebuffer			= beginInfo.pFramebuffer->vkFramebuffer;
		renderPassInfo.renderArea			= beginInfo.renderArea;
		renderPassInfo.clearValueCount		= beginInfo.clearValueCount;
		renderPassInfo.pClearValues			= beginInfo.pClearValues;

		vkCmdBeginRenderPass(mpCommandBuffer->vkCommandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE); // Only inline for now
	}

	void VulkanCommandRecorder::EndRenderpass()
	{
		vkCmdEndRenderPass(mpCommandBuffer->vkCommandBuffer);
	}

	void VulkanCommandRecorder::BeginRendering(const VulkanRenderingBeginInfo& beginInfo)
	{
		constexpr const uSize maxColorAttachmentSize = 16;

		VkRenderingInfoKHR vkRenderInfo = {};
		vkRenderInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO_KHR;

		VkRenderingAttachmentInfoKHR vkColorAttachments[maxColorAttachmentSize] = {};
		VkRenderingAttachmentInfoKHR vkDepthAttachment = {};
		VkRenderingAttachmentInfoKHR vkStencilAttachment = {};

		if (beginInfo.colorAttachmentCount > maxColorAttachmentSize) // @TODO: should be debug_assert
		{
			LogError("BeginRendering failed. colorAttachmentCount > maxColorAttachmentSize");
		}

		for (uSize i = 0; i < beginInfo.colorAttachmentCount; i++)
		{
			vkColorAttachments[i].sType					= VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO_KHR;
			vkColorAttachments[i].imageView				= beginInfo.pColorAttachments[i].pImageView->vkImageView;
			vkColorAttachments[i].imageLayout			= beginInfo.pColorAttachments[i].imageLayout;
			vkColorAttachments[i].loadOp				= beginInfo.pColorAttachments[i].loadOp;
			vkColorAttachments[i].storeOp				= beginInfo.pColorAttachments[i].storeOp;
			vkColorAttachments[i].clearValue			= beginInfo.pColorAttachments[i].clearValue;
			vkColorAttachments[i].resolveMode			= VK_RESOLVE_MODE_NONE;
			vkColorAttachments[i].resolveImageLayout	= VK_IMAGE_LAYOUT_UNDEFINED;
			vkColorAttachments[i].resolveImageView		= VK_NULL_HANDLE;
			vkColorAttachments[i].pNext					= nullptr;
		}

		vkRenderInfo.pColorAttachments = vkColorAttachments;
		vkRenderInfo.colorAttachmentCount = beginInfo.colorAttachmentCount;

		if (beginInfo.pDepthAttachment)
		{
			vkDepthAttachment.sType					= VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO_KHR;
			vkDepthAttachment.imageView				= beginInfo.pDepthAttachment->pImageView->vkImageView;
			vkDepthAttachment.imageLayout			= beginInfo.pDepthAttachment->imageLayout;
			vkDepthAttachment.loadOp				= beginInfo.pDepthAttachment->loadOp;
			vkDepthAttachment.storeOp				= beginInfo.pDepthAttachment->storeOp;
			vkDepthAttachment.clearValue			= beginInfo.pDepthAttachment->clearValue;
			vkDepthAttachment.resolveMode			= VK_RESOLVE_MODE_NONE;
			vkDepthAttachment.resolveImageLayout	= VK_IMAGE_LAYOUT_UNDEFINED;
			vkDepthAttachment.resolveImageView		= VK_NULL_HANDLE;
			vkDepthAttachment.pNext					= nullptr;

			vkRenderInfo.pDepthAttachment = &vkDepthAttachment;
		}
		
		if (beginInfo.pStencilAttachment)
		{
			vkStencilAttachment.sType				= VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO_KHR;
			vkStencilAttachment.imageView			= beginInfo.pStencilAttachment->pImageView->vkImageView;
			vkStencilAttachment.imageLayout			= beginInfo.pStencilAttachment->imageLayout;
			vkStencilAttachment.loadOp				= beginInfo.pStencilAttachment->loadOp;
			vkStencilAttachment.storeOp				= beginInfo.pStencilAttachment->storeOp;
			vkStencilAttachment.clearValue			= beginInfo.pStencilAttachment->clearValue;
			vkStencilAttachment.resolveMode			= VK_RESOLVE_MODE_NONE;
			vkStencilAttachment.resolveImageLayout	= VK_IMAGE_LAYOUT_UNDEFINED;
			vkStencilAttachment.resolveImageView	= VK_NULL_HANDLE;
			vkStencilAttachment.pNext				= nullptr;

			vkRenderInfo.pStencilAttachment = &vkStencilAttachment;
		}
	
		vkRenderInfo.renderArea	= beginInfo.renderArea;
		vkRenderInfo.viewMask	= 0;
		vkRenderInfo.layerCount	= 1;
		vkRenderInfo.flags		= 0;
		vkRenderInfo.pNext		= nullptr;

		PFN_vkCmdBeginRenderingKHR vkCmdBeginRenderingKHR2 =
			(PFN_vkCmdBeginRenderingKHR)vkGetDeviceProcAddr(mpCommandBuffer->pDevice->vkDevice, "vkCmdBeginRenderingKHR");

		vkCmdBeginRenderingKHR2(mpCommandBuffer->vkCommandBuffer, &vkRenderInfo);
	}

	void VulkanCommandRecorder::EndRendering()
	{
		PFN_vkCmdEndRenderingKHR vkCmdEndRenderingKHR2 =
			(PFN_vkCmdEndRenderingKHR)vkGetDeviceProcAddr(mpCommandBuffer->pDevice->vkDevice, "vkCmdEndRenderingKHR");

		vkCmdEndRenderingKHR2(mpCommandBuffer->vkCommandBuffer);
	}

	void VulkanCommandRecorder::SetGraphicsPipeline(VulkanGraphicsPipeline* pPipeline)
	{
		vkCmdBindPipeline(mpCommandBuffer->vkCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pPipeline->vkPipeline);
	}

	void VulkanCommandRecorder::SetComputePipeline(VulkanComputePipeline* pPipeline)
	{
		vkCmdBindPipeline(mpCommandBuffer->vkCommandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, pPipeline->vkPipeline);
	}

	void VulkanCommandRecorder::SetVertexBuffers(VulkanBufferBind* pBuffers, uSize bufferCount)
	{
		constexpr const uSize maxVertexBufferSize = 16;

		VkBuffer		vkBuffers[maxVertexBufferSize] = {};
		VkDeviceSize	vkOffsetSizes[maxVertexBufferSize] = {};

		if (bufferCount > maxVertexBufferSize) // @TODO: should be debug_assert
		{
			LogError("SetVertexBuffers failed. buffers.Size() > maxVertexBufferSize");
		}

		for (uSize i = 0; i < bufferCount; i++)
		{
			vkBuffers[i]		= pBuffers[i].pBuffer->vkBuffer;
			vkOffsetSizes[i]	= pBuffers[i].offset;
		}

		vkCmdBindVertexBuffers(mpCommandBuffer->vkCommandBuffer, 0, bufferCount, vkBuffers, vkOffsetSizes);
	}

	void VulkanCommandRecorder::SetIndexBuffer(VulkanBuffer* pIndexBuffer, uSize offset, VkIndexType indexType)
	{
		vkCmdBindIndexBuffer(mpCommandBuffer->vkCommandBuffer, pIndexBuffer->vkBuffer, offset, indexType);
	}

	void VulkanCommandRecorder::BindUniforms(VulkanGraphicsPipeline* pPipeline, uInt32 set,
		VulkanUniformBufferBind* pBufferBinds, uSize bufferCount,
		VulkanUniformImageBind* pImageBinds, uSize imageCount)
	{
		constexpr const uSize maxWriteDescriptorSets = 16;

		VkWriteDescriptorSet	writeDescriptorSets[maxWriteDescriptorSets] = {};
		VkDescriptorBufferInfo	bufferInfos[maxWriteDescriptorSets] = {};
		VkDescriptorImageInfo	imageInfos[maxWriteDescriptorSets] = {};

		if (bufferCount + imageCount > maxWriteDescriptorSets) // @TODO: should be debug_assert
		{
			LogError("BindUniforms failed. bindingCount > maxWriteDescriptorSets");
		}

		for (uSize i = 0; i < bufferCount; i++)
		{
			bufferInfos[i].buffer	= pBufferBinds[i].pBuffer->vkBuffer;
			bufferInfos[i].offset	= pBufferBinds[i].offset;
			bufferInfos[i].range	= pBufferBinds[i].range;

			writeDescriptorSets[i].sType			= VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			writeDescriptorSets[i].dstSet			= 0; // Ignored
			writeDescriptorSets[i].dstBinding		= pBufferBinds[i].binding;
			writeDescriptorSets[i].descriptorCount	= 1;
			writeDescriptorSets[i].descriptorType	= VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			writeDescriptorSets[i].pBufferInfo		= &bufferInfos[i];
		}

		for (uSize i = 0; i < imageCount; i++)
		{
			imageInfos[i].sampler		= pImageBinds[i].vkSampler;
			imageInfos[i].imageView		= pImageBinds[i].pImageView->vkImageView;
			imageInfos[i].imageLayout	= pImageBinds[i].vkLayout;

			writeDescriptorSets[bufferCount + i].sType				= VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			writeDescriptorSets[bufferCount + i].dstSet				= 0; // Ignored
			writeDescriptorSets[bufferCount + i].dstBinding			= pImageBinds[i].binding;
			writeDescriptorSets[bufferCount + i].descriptorCount	= 1;
			writeDescriptorSets[bufferCount + i].descriptorType		= VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			writeDescriptorSets[bufferCount + i].pImageInfo			= &imageInfos[i];
		}

		PFN_vkCmdPushDescriptorSetKHR vkCmdPushDescriptorSetKHR2 = 
			(PFN_vkCmdPushDescriptorSetKHR)vkGetDeviceProcAddr(pPipeline->pDevice->vkDevice, "vkCmdPushDescriptorSetKHR");

		vkCmdPushDescriptorSetKHR2(mpCommandBuffer->vkCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, 
			pPipeline->vkPipelineInfo.layout, set, bufferCount + imageCount, writeDescriptorSets);
	}

	void VulkanCommandRecorder::SetViewport(const VkViewport& viewport, const VkRect2D scissor)
	{
		vkCmdSetViewport(mpCommandBuffer->vkCommandBuffer, 0, 1, &viewport);
		vkCmdSetScissor(mpCommandBuffer->vkCommandBuffer, 0, 1, &scissor);
	}

	void VulkanCommandRecorder::DrawIndexed(uInt32 instanceCount, uInt32 indexCount, uInt32 indexStart, uInt32 vertexOffset)
	{
		vkCmdDrawIndexed(mpCommandBuffer->vkCommandBuffer, indexCount, instanceCount, indexStart, vertexOffset, 0);
	}

	void VulkanCommandRecorder::CopyBuffer(VulkanBuffer* pSrcBuffer, VulkanBuffer* pDestBuffer, uSize sizeBytes, uSize srcOffset, uSize destOffset)
	{
		VkBufferCopy copyRegion = {};
		copyRegion.srcOffset	= srcOffset;
		copyRegion.dstOffset	= destOffset;
		copyRegion.size			= sizeBytes;

		vkCmdCopyBuffer(mpCommandBuffer->vkCommandBuffer, pSrcBuffer->vkBuffer, pDestBuffer->vkBuffer, 1, &copyRegion);
	}

	void VulkanCommandRecorder::CopyBufferToImage(VulkanBuffer* pSrcBuffer, VulkanImage* pDestImage,
		VkImageLayout vkLayout, const Array<VkBufferImageCopy>& regions)
	{
		vkCmdCopyBufferToImage(mpCommandBuffer->vkCommandBuffer, pSrcBuffer->vkBuffer, pDestImage->vkImage, vkLayout, regions.Size(), regions.Data());
	}

	void VulkanCommandRecorder::PipelineBarrier(const VulkanPipelineBarrierInfo& barrierInfo)
	{
		vkCmdPipelineBarrier(
			mpCommandBuffer->vkCommandBuffer,
			barrierInfo.srcStage,
			barrierInfo.dstStage,
			barrierInfo.dependencyFlags,
			barrierInfo.memoryBarrierCount,
			barrierInfo.pMemoryBarriers,
			barrierInfo.bufferMemoryBarrierCount,
			barrierInfo.pBufferMemoryBarriers,
			barrierInfo.imageMemoryBarrierCount,
			barrierInfo.pImageMemoryBarriers);
	}

	void VulkanCommandRecorder::PipelineBarrierSwapchainImageBegin(VulkanImage* pSwapchainImage)
	{
		VkImageMemoryBarrier vkImageMemoryBarrier = {};
		vkImageMemoryBarrier.sType								= VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		vkImageMemoryBarrier.srcAccessMask						= 0;
		vkImageMemoryBarrier.dstAccessMask						= VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		vkImageMemoryBarrier.oldLayout							= VK_IMAGE_LAYOUT_UNDEFINED;
		vkImageMemoryBarrier.newLayout							= VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		vkImageMemoryBarrier.image								= pSwapchainImage->vkImage;
		vkImageMemoryBarrier.subresourceRange.aspectMask		= VK_IMAGE_ASPECT_COLOR_BIT;
		vkImageMemoryBarrier.subresourceRange.baseMipLevel		= 0;
		vkImageMemoryBarrier.subresourceRange.levelCount		= 1;
		vkImageMemoryBarrier.subresourceRange.baseArrayLayer	= 0;
		vkImageMemoryBarrier.subresourceRange.layerCount		= 1;

		VulkanPipelineBarrierInfo barrierInfo = {};
		barrierInfo.srcStage					= VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		barrierInfo.dstStage					= VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		barrierInfo.dependencyFlags				= 0;
		barrierInfo.memoryBarrierCount			= 0;
		barrierInfo.pMemoryBarriers				= nullptr;
		barrierInfo.bufferMemoryBarrierCount	= 0;
		barrierInfo.pBufferMemoryBarriers		= nullptr;
		barrierInfo.imageMemoryBarrierCount		= 1;
		barrierInfo.pImageMemoryBarriers		= &vkImageMemoryBarrier;

		vkCmdPipelineBarrier(
			mpCommandBuffer->vkCommandBuffer,
			barrierInfo.srcStage,
			barrierInfo.dstStage,
			barrierInfo.dependencyFlags,
			barrierInfo.memoryBarrierCount,
			barrierInfo.pMemoryBarriers,
			barrierInfo.bufferMemoryBarrierCount,
			barrierInfo.pBufferMemoryBarriers,
			barrierInfo.imageMemoryBarrierCount,
			barrierInfo.pImageMemoryBarriers);
	}

	void VulkanCommandRecorder::PipelineBarrierSwapchainImageEnd(VulkanImage* pSwapchainImage)
	{
		VkImageMemoryBarrier vkImageMemoryBarrier = {};
		vkImageMemoryBarrier.sType								= VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		vkImageMemoryBarrier.srcAccessMask						= VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		vkImageMemoryBarrier.dstAccessMask						= 0;
		vkImageMemoryBarrier.oldLayout							= VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		vkImageMemoryBarrier.newLayout							= VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
		vkImageMemoryBarrier.image								= pSwapchainImage->vkImage;
		vkImageMemoryBarrier.subresourceRange.aspectMask		= VK_IMAGE_ASPECT_COLOR_BIT;
		vkImageMemoryBarrier.subresourceRange.baseMipLevel		= 0;
		vkImageMemoryBarrier.subresourceRange.levelCount		= 1;
		vkImageMemoryBarrier.subresourceRange.baseArrayLayer	= 0;
		vkImageMemoryBarrier.subresourceRange.layerCount		= 1;

		VulkanPipelineBarrierInfo barrierInfo = {};
		barrierInfo.srcStage					= VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		barrierInfo.dstStage					= VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
		barrierInfo.dependencyFlags				= 0;
		barrierInfo.memoryBarrierCount			= 0;
		barrierInfo.pMemoryBarriers				= nullptr;
		barrierInfo.bufferMemoryBarrierCount	= 0;
		barrierInfo.pBufferMemoryBarriers		= nullptr;
		barrierInfo.imageMemoryBarrierCount		= 1;
		barrierInfo.pImageMemoryBarriers		= &vkImageMemoryBarrier;

		vkCmdPipelineBarrier(
			mpCommandBuffer->vkCommandBuffer,
			barrierInfo.srcStage,
			barrierInfo.dstStage,
			barrierInfo.dependencyFlags,
			barrierInfo.memoryBarrierCount,
			barrierInfo.pMemoryBarriers,
			barrierInfo.bufferMemoryBarrierCount,
			barrierInfo.pBufferMemoryBarriers,
			barrierInfo.imageMemoryBarrierCount,
			barrierInfo.pImageMemoryBarriers);
	}
}