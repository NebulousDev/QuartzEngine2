#pragma once

#include "VulkanRenderScene.h"

namespace Quartz
{
	struct VulkanGraphics;

	struct VulkanState
	{
		Array<VulkanRenderScene> renderScenes;
	};

	bool CreateVulkanState(const VulkanGraphics* pGraphics, VulkanState* pState);
	void DestroyVulkanState(VulkanState* pState);

	void AddRenderScene(VulkanState* pState, const VulkanRenderScene& renderScene);
}