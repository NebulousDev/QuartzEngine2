#pragma once

#include "Graphics/FrameGraph/FrameGraph.h"
#include "Vulkan/VulkanGraphics.h"

namespace Quartz
{
	struct VulkanFrameGraphState
	{
		VulkanCommandPool* pGraphicsPool;
		VulkanCommandPool* pComputePool;
		VulkanCommandPool* pTransferPool;
	};

	void SetupVulkanFrameGraph(VulkanGraphics& graphics);
	void SetupVulkanFrameGraphFunctions(VulkanGraphics& graphics, FrameGraph::FrameGraphFunctions& outFunctions);
}