#include "Vulkan/Renderers/VulkanSceneRenderer.h"

#include "Component/MeshComponent.h"
#include "Component/MaterialComponent.h"

namespace Quartz
{
	void VulkanSceneRenderer::Initialize(VulkanGraphics& graphics, VulkanDevice& device, VulkanShaderCache& shaderCache,
		VulkanPipelineCache& pipelineCache, uSize maxInFlightCount)
	{
		mpGraphics	= &graphics;
		mpDevice	= &device;

		VulkanResourceManager*	pResources	= graphics.pResourceManager;
		VulkanDevice*			pDevice		= graphics.pPrimaryDevice;

		VulkanShader* pVertexShader = shaderCache.FindOrCreateShader("Shaders/default.vert", VK_SHADER_STAGE_VERTEX_BIT);
		VulkanShader* pFragmentShader = shaderCache.FindOrCreateShader("Shaders/default.frag", VK_SHADER_STAGE_FRAGMENT_BIT);

		Array<VulkanAttachment, 2> attachments =
		{
			{ "Swapchain",		VULKAN_ATTACHMENT_TYPE_SWAPCHAIN,		VK_FORMAT_B8G8R8A8_UNORM },
			{ "Depth-Stencil",	VULKAN_ATTACHMENT_TYPE_DEPTH_STENCIL,	VK_FORMAT_D24_UNORM_S8_UINT }
		};

		/*
		VkVertexInputBindingDescription vertexBufferAttachment = {};
		vertexBufferAttachment.binding		= 0;
		vertexBufferAttachment.stride		= 6 * sizeof(float);
		vertexBufferAttachment.inputRate	= VK_VERTEX_INPUT_RATE_VERTEX;

		Array<VkVertexInputBindingDescription> vertexBindings;

		vertexBindings.PushBack(vertexBufferAttachment);

		VkVertexInputAttributeDescription positionAttrib = {};
		positionAttrib.binding				= 0;
		positionAttrib.location				= 0;
		positionAttrib.format				= VK_FORMAT_R32G32B32_SFLOAT;
		positionAttrib.offset				= 0;

		VkVertexInputAttributeDescription normalAttrib = {};
		normalAttrib.binding				= 0;
		normalAttrib.location				= 1;
		normalAttrib.format					= VK_FORMAT_R32G32B32_SFLOAT;
		normalAttrib.offset					= 3 * sizeof(float);

		VkVertexInputAttributeDescription tangentAttrib = {};
		tangentAttrib.binding				= 0;
		tangentAttrib.location				= 2;
		tangentAttrib.format				= VK_FORMAT_R32G32B32_SFLOAT;
		tangentAttrib.offset				= 6 * sizeof(float);

		VkVertexInputAttributeDescription texCoordAttrib = {};
		texCoordAttrib.binding				= 0;
		texCoordAttrib.location				= 3;
		texCoordAttrib.format				= VK_FORMAT_R32G32_SFLOAT;
		texCoordAttrib.offset				= 9 * sizeof(float);

		Array<VkVertexInputAttributeDescription> vertexAttributes;

		vertexAttributes.PushBack(positionAttrib);
		vertexAttributes.PushBack(normalAttrib);

		VulkanGraphicsPipelineInfo pipelineInfo = 
			pipelineCache.MakeGraphicsPipelineInfo(
			{ pVertexShader, pFragmentShader },
			attachments, vertexAttributes, vertexBindings
		);
		*/

		VulkanGraphicsPipelineInfo pipelineInfo =
			pipelineCache.MakeGraphicsPipelineInfo(
				{ pVertexShader, pFragmentShader }, attachments);

		pipelineInfo.vkFrontFace = VK_FRONT_FACE_CLOCKWISE;
		pipelineInfo.vkCullMode = VK_CULL_MODE_NONE;

		mpDefaultPipeline = pipelineCache.FindOrCreateGraphicsPipeline(pipelineInfo);

		if (!mpDefaultPipeline)
		{
			LogFatal("Failed to create Pipeline!");
		}
	}

	void VulkanSceneRenderer::Update(EntityWorld& world, VulkanBufferCache& bufferCache,
		VulkanShaderCache& shaderCache, VulkanPipelineCache& pipelineCache,
		CameraComponent& camera, TransformComponent& cameraTransform, uSize frameIdx)
	{
		auto& renderableView = world.CreateView<MeshComponent, TransformComponent>();

		mRenderables.Clear();
		bufferCache.ResetPerModelBuffers();

		for (Entity& entity : renderableView)
		{
			MeshComponent& meshComponent			= world.Get<MeshComponent>(entity);
			TransformComponent& transformComponent	= world.Get<TransformComponent>(entity);

			VulkanRenderable renderable = {};

			bool vertexDataFound;
			bufferCache.FillRenderableVertexData(renderable, meshComponent.modelURIHash, &meshComponent.modelData, vertexDataFound);

			VulkanRenderablePerModelUBO perModelUbo = {};
			perModelUbo.model	= transformComponent.GetMatrix();
			perModelUbo.view	= cameraTransform.GetViewMatrix();
			perModelUbo.proj	= camera.GetProjectionMatrix();

			bufferCache.FillRenderablePerModelData(renderable, 0, &perModelUbo, sizeof(VulkanRenderablePerModelUBO));

			if (world.HasComponent<MaterialComponent>(entity))
			{
				MaterialComponent& materialComponent = world.Get<MaterialComponent>(entity);

				VulkanShader* pVertexShader		= shaderCache.FindOrCreateShader(materialComponent.vertexURI, VK_SHADER_STAGE_VERTEX_BIT);
				VulkanShader* pFragmentShader	= shaderCache.FindOrCreateShader(materialComponent.fragmentURI, VK_SHADER_STAGE_FRAGMENT_BIT);

				Array<VulkanAttachment, 2> attachments =
				{
					{ "Swapchain",		VULKAN_ATTACHMENT_TYPE_SWAPCHAIN,		VK_FORMAT_B8G8R8A8_UNORM },
					{ "Depth-Stencil",	VULKAN_ATTACHMENT_TYPE_DEPTH_STENCIL,	VK_FORMAT_D24_UNORM_S8_UINT }
				};

				VulkanGraphicsPipelineInfo pipelineInfo = pipelineCache.MakeGraphicsPipelineInfo(
					{ pVertexShader, pFragmentShader }, attachments);

				pipelineInfo.vkFrontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
				pipelineInfo.vkCullMode = VK_CULL_MODE_NONE;

				renderable.pPipeline = pipelineCache.FindOrCreateGraphicsPipeline(pipelineInfo);
			}
			else
			{
				renderable.pPipeline = mpDefaultPipeline;
			}

			mRenderables.PushBack(renderable);
		}
	}

	void VulkanSceneRenderer::RecordTransfers(VulkanCommandRecorder& transferRecorder, VulkanBufferCache& bufferCache, uSize frameIdx)
	{
		bufferCache.RecordTransfers(transferRecorder);
	}

	void VulkanSceneRenderer::RecordDraws(VulkanCommandRecorder& recorder, uSize frameIdx)
	{
		for (VulkanRenderable& renderable : mRenderables)
		{
			recorder.SetGraphicsPipeline(renderable.pPipeline);

			recorder.SetIndexBuffer(renderable.meshLocation.pIndexBuffer->GetVulkanBuffer(),
				renderable.meshLocation.indexEntry.offset, VK_INDEX_TYPE_UINT16);

			VulkanBufferBind pVertexBufferBinds[] = 
			{ 
				{renderable.meshLocation.pVertexBuffer->GetVulkanBuffer(), renderable.meshLocation.vertexEntry.offset} 
			};

			recorder.SetVertexBuffers(pVertexBufferBinds, 1);

			VulkanUniformBufferBind binding = {};
			binding.binding = 0;
			binding.pBuffer = renderable.perModelLocation.pPerModelBuffer->GetVulkanBuffer();
			binding.offset	= renderable.perModelLocation.perModelEntry.offset;
			binding.range	= renderable.perModelLocation.perModelEntry.sizeBytes;

			VulkanUniformBufferBind pBufferBinds[] = { binding };
			
			recorder.BindUniforms(renderable.pPipeline, 0, pBufferBinds, 1, nullptr, 0);

			recorder.DrawIndexed(1, renderable.indexCount, 0, 0); //renderable.meshLocation.indexEntry.offset / sizeof(uInt16)
		}
	}
}