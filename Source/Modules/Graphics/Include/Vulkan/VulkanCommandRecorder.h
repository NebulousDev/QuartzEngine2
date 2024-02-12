#pragma once

#include "GfxAPI.h"
#include "Primatives/VulkanImage.h"
#include "Primatives/VulkanCommandBuffer.h"

namespace Quartz
{
	struct VulkanRenderpassBeginInfo
	{
		VulkanFramebuffer*	pFramebuffer;
		VkRect2D			renderArea;
		VkClearValue*		pClearValues;
		uSize				clearValueCount;
	};

	struct VulkanRenderingAttachmentInfo
	{
		VulkanImageView*         pImageView;
		VkImageLayout            imageLayout;
		VkAttachmentLoadOp       loadOp;
		VkAttachmentStoreOp      storeOp;
		VkClearValue             clearValue;
	};

	struct VulkanRenderingBeginInfo
	{
		VulkanRenderingAttachmentInfo*	pColorAttachments;
		uSize							colorAttachmentCount;
		VulkanRenderingAttachmentInfo*	pDepthAttachment;
		VulkanRenderingAttachmentInfo*	pStencilAttachment;
		VkRect2D						renderArea;
	};

	struct VulkanPipelineBarrierInfo
	{
		VkPipelineStageFlags	srcStage;
		VkPipelineStageFlags	dstStage;
		VkDependencyFlags		dependencyFlags;
		uInt32					memoryBarrierCount;
		VkMemoryBarrier*		pMemoryBarriers;
		uInt32					bufferMemoryBarrierCount;
		VkBufferMemoryBarrier*	pBufferMemoryBarriers;
		uInt32					imageMemoryBarrierCount;
		VkImageMemoryBarrier*	pImageMemoryBarriers;
	};

	struct VulkanUniformBind
	{
		uInt32			binding;
		VulkanBuffer*	pBuffer;
		uSize			offset;
		uSize			range;
	};

	struct VulkanBufferBind
	{
		VulkanBuffer*	pBuffer;
		uSize			offset;
	};

	class QUARTZ_GRAPHICS_API VulkanCommandRecorder
	{
	private:
		VulkanCommandBuffer* mpCommandBuffer;

	public:
		VulkanCommandRecorder(VulkanCommandBuffer* pCommandBuffer);

		void BeginRecording();
		void EndRecording();

		void BeginRenderpass(VulkanRenderpass* pRenderpass, const VulkanRenderpassBeginInfo& beginInfo);
		void EndRenderpass();

		void BeginRendering(const VulkanRenderingBeginInfo& beginInfo);
		void EndRendering();

		void SetGraphicsPipeline(VulkanGraphicsPipeline* pPipeline);
		void SetComputePipeline(VulkanComputePipeline* pPipeline);

		void SetVertexBuffers(VulkanBufferBind* pBuffers, uSize bufferCount);
		void SetIndexBuffer(VulkanBuffer* pIndexBuffer, uSize offset, VkIndexType indexType);

		void BindUniforms(VulkanGraphicsPipeline* pPipeline, uInt32 set, VulkanUniformBind* pBindings, uSize bindingCount);

		void DrawIndexed(uInt32 instanceCount, uInt32 indexCount, uInt32 indexStart);

		void CopyBuffer(VulkanBuffer* pSrcBuffer, VulkanBuffer* pDestBuffer, 
			uSize sizeBytes, uSize srcOffset, uSize destOffset);

		void PipelineBarrier(const VulkanPipelineBarrierInfo& barrierInfo);
		void PipelineBarrierSwapchainImageBegin(VulkanImage* pSwapchainImage);
		void PipelineBarrierSwapchainImageEnd(VulkanImage* pSwapchainImage);
	};
}