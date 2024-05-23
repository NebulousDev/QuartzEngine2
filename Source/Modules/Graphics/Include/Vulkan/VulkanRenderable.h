#pragma once

#include "VulkanMultiBuffer.h"
#include "Primatives/VulkanPipeline.h"
#include "Types/Types.h"

namespace Quartz
{
	struct MeshBufferLocation
	{
		VulkanMultiBufferEntry	vertexEntry;
		VulkanMultiBufferEntry	indexEntry;
		VulkanMultiBuffer*		pVertexBuffer;
		VulkanMultiBuffer*		pIndexBuffer;
	};

	struct UniformBufferLocation
	{
		VulkanMultiBufferEntry	entry;
		VulkanMultiBuffer*		pBuffer;
	};

	struct VulkanRenderable
	{
		MeshBufferLocation		meshBuffer;
		UniformBufferLocation	transformBuffer;
		UniformBufferLocation	materialBuffer;
		VulkanGraphicsPipeline* pPipeline;
		float					distFromCamera;
		uInt32					indexStart;
		uInt32					indexCount;
		VkIndexType				vkIndexType;
		Array<VulkanUniformImageBind, 16>	imageBinds;

		//temp
		bool					isTerrain;
	};
}