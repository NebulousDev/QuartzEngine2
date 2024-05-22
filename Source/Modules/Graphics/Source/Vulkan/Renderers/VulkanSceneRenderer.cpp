#include "Vulkan/Renderers/VulkanSceneRenderer.h"

#include "Component/MeshComponent.h"
#include "Component/MaterialComponent.h"

#include "Engine.h"

namespace Quartz
{
	void VulkanSceneRenderer::Initialize(VulkanGraphics& graphics, VulkanDevice& device, VulkanShaderCache& shaderCache,
		VulkanPipelineCache& pipelineCache, uSize maxInFlightCount)
	{
		mpGraphics	= &graphics;
		mpDevice	= &device;

		VulkanResourceManager*	pResources	= graphics.pResourceManager;
		VulkanDevice*			pDevice		= graphics.pPrimaryDevice;

		VulkanShader* pVertexShader = shaderCache.FindOrCreateShader("Shaders/default.qsvert");
		VulkanShader* pFragmentShader = shaderCache.FindOrCreateShader("Shaders/default.qsfrag");

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

			Model* pModel = meshComponent.pCachedModel;

			if (!pModel)
			{
				pModel = Engine::GetAssetManager().GetOrLoadAsset<Model>(meshComponent.modelURI);

				if (!pModel)
				{
					// Failed to load model
					continue;
				}

				meshComponent.pCachedModel = pModel;
			}

			bool vertexDataFound;
			MeshBufferLocation bufferLocation;
			MeshBufferLocation stagingBufferLocation;
			bufferCache.GetOrAllocateBuffers(*pModel, bufferLocation, stagingBufferLocation, 1, 4, vertexDataFound);
			// @TODO: error check ^

			for (const Mesh& mesh : pModel->meshes)
			{
				VulkanRenderable renderable = {};

				const IndexElement indexElement	= mesh.indexElement;
				const IndexFormat indexType		= indexElement.format;
				const uInt32 indexSize			= indexElement.FormatSize();

				renderable.vkIndexType = indexType == INDEX_FORMAT_UINT16 ? VK_INDEX_TYPE_UINT16 : VK_INDEX_TYPE_UINT32;

				renderable.meshLocation = bufferLocation;
				renderable.indexStart	= mesh.indicesStartBytes / indexSize;
				renderable.indexCount	= mesh.indicesSizeBytes / indexSize; 

				VulkanRenderablePerModelUBO perModelUbo = {};
				perModelUbo.model	= transformComponent.GetMatrix();
				perModelUbo.view	= cameraTransform.GetViewMatrix();
				perModelUbo.proj	= camera.GetProjectionMatrix();

				// @TODO: This can probably be optimized by moving out of the mesh and into the model
				bufferCache.FillRenderablePerModelData(renderable, 0, &perModelUbo, sizeof(VulkanRenderablePerModelUBO));

				if (world.HasComponent<MaterialComponent>(entity))
				{
					MaterialComponent& materialComponent = world.Get<MaterialComponent>(entity);

					VulkanShader* pVertexShader		= shaderCache.FindOrCreateShader(materialComponent.vertexURI);
					VulkanShader* pFragmentShader	= shaderCache.FindOrCreateShader(materialComponent.fragmentURI);

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
				renderable.meshLocation.indexEntry.offset, renderable.vkIndexType);

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

			recorder.DrawIndexed(1, renderable.indexCount, renderable.indexStart, 0);
		}
	}
}