#include "SkyRenderer.h"

namespace Quartz
{
	void VulkanSkyRenderer::Initialize(VulkanGraphics& graphics, const AtmosphereProperties& atmosphere, VulkanShaderCache& shaderCache, VulkanPipelineCache& pipelineCache)
	{
		mAtmosphere = atmosphere;

		VulkanShader* pSkyVertexShader = shaderCache.FindOrCreateShader("Shaders/sky.vert", VK_SHADER_STAGE_VERTEX_BIT);
		VulkanShader* pSkyFragmentShader = shaderCache.FindOrCreateShader("Shaders/sky.frag", VK_SHADER_STAGE_FRAGMENT_BIT);

		Array<VulkanAttachment> attachments =
		{
			{ "Swapchain",		VULKAN_ATTACHMENT_TYPE_SWAPCHAIN,		VK_FORMAT_B8G8R8A8_UNORM },
			{ "Depth-Stencil",	VULKAN_ATTACHMENT_TYPE_DEPTH_STENCIL,	VK_FORMAT_D24_UNORM_S8_UINT }
		};

		// All geometry will be generated in the vertex shader
		mpSkyRenderPipeline = pipelineCache.FindOrCreateGraphicsPipeline(
			{ pSkyVertexShader, pSkyFragmentShader }, attachments, {}, {} );
	}

	void VulkanSkyRenderer::RecordTransfers(VulkanCommandRecorder& transferRecorder)
	{

	}

	void VulkanSkyRenderer::RecordDraws(VulkanCommandRecorder& renderRecorder)
	{
		renderRecorder.SetGraphicsPipeline(mpSkyRenderPipeline);
		renderRecorder.Draw(1, 3, 0);
	}
}

