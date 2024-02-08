#pragma once

#include "GfxDLL.h"
#include "VulkanCommandBuffer.h"

namespace Quartz
{
	struct VulkanRenderpassBeginInfo
	{
		VulkanFramebuffer*	pFramebuffer;
		VkRect2D			renderArea;
		Array<VkClearValue>	clearValues;
	};

	struct VulkanUniformBinding
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

		void SetGraphicsPipeline(VulkanGraphicsPipeline* pPipeline);
		void SetComputePipeline(VulkanComputePipeline* pPipeline);

		void SetVertexBuffers(const Array<VulkanBufferBind>& buffers);
		void SetIndexBuffer(VulkanBuffer* pIndexBuffer, uSize offset, VkIndexType indexType);

		void BindUniforms(VulkanGraphicsPipeline* pPipeline, uInt32 set, const Array<VulkanUniformBinding>& bindings);

		//void BindUniform(UInt32 set, UInt32 binding, Uniform* pUniform, UInt32 element) = 0;
		//void BindUniformTexture(UInt32 set, UInt32 binding, UniformTextureSampler* pUniformTextureSampler) = 0;

		void DrawIndexed(uInt32 instanceCount, uInt32 indexCount, uInt32 indexStart);

		void CopyBuffer(VulkanBuffer* pSrcBuffer, VulkanBuffer* pDestBuffer, 
			uSize sizeBytes, uSize offsetSrc, uSize offsetDest);
	};
}