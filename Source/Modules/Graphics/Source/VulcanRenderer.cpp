#include "VulkanRenderer.h"

#include "Log.h"
#include "Vulkan/VulkanGraphics.h"

namespace Quartz
{
	void VulkanRenderer::Initialize(VulkanGraphics* pGraphics)
	{
		mpGraphics	= pGraphics;
		mpSwapchain = pGraphics->pResourceManager->CreateSwapchain(
			&pGraphics->primaryDevice, pGraphics->pSurface, 3);
	}

	void VulkanRenderer::RenderScene(VulkanRenderScene* pRenderScene)
	{

	}

	void VulkanRenderer::RenderUpdate(Runtime* pRuntime, double delta)
	{
		RenderScene(nullptr);
	}

	void VulkanRenderer::Register(Runtime* pRuntime)
	{
		pRuntime->RegisterOnUpdate(&VulkanRenderer::RenderUpdate, this);
	}
}