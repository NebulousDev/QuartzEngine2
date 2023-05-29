#pragma once

#include "GfxDLL.h"
#include "Runtime/Runtime.h"

#include "Vulkan/VulkanGraphics.h"
#include "Vulkan/VulkanRenderScene.h"
#include "Vulkan/VulkanSwapchain.h"

namespace Quartz
{
	class QUARTZ_GRAPHICS_API VulkanRenderer
	{
	private:
		VulkanGraphics*		mpGraphics;
		VulkanSwapchain*	mpSwapchain;

	public:
		void Initialize(VulkanGraphics* pGraphics);

		void RenderScene(VulkanRenderScene* pRenderScene);

		void RenderUpdate(Runtime* pRuntime, double delta);
		void Register(Runtime* pRuntime);
	};
}