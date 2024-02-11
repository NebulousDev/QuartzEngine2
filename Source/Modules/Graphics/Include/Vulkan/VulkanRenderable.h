#pragma once

#include "VulkanMultiBuffer.h"
#include "Types/Types.h"

namespace Quartz
{
	struct MeshBufferLocation
	{
		VulkanMultiBufferEntry	vertexEntry;
		VulkanMultiBufferEntry	indexEntry;
		uInt32					vertexBufferIndex;
		uInt32					indexBufferIndex;
	};

	struct PerModelBufferLocation
	{
		VulkanMultiBufferEntry	perModelEntry;
		uInt32					perModelBufferIndex;
	};

	struct VulkanRenderable
	{
		MeshBufferLocation		meshLocation;
		PerModelBufferLocation	perModelLocation;
		uInt32					indexCount;
		uInt32					materialId;
	};
}