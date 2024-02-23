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

	struct PerModelBufferLocation
	{
		VulkanMultiBufferEntry	perModelEntry;
		VulkanMultiBuffer*		pPerModelBuffer;
	};

	struct VulkanRenderable
	{
		MeshBufferLocation		meshLocation;
		PerModelBufferLocation	perModelLocation;
		VulkanGraphicsPipeline* pPipeline;
		float					distance;
		uInt32					indexStart;
		uInt32					indexCount;

		//temp
		bool					isTerrain;
	};
}