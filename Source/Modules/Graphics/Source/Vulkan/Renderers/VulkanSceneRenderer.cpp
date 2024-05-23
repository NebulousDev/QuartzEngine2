#include "Vulkan/Renderers/VulkanSceneRenderer.h"

#include "Resource/Assets/Shader.h"
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

		VulkanShader* pVertexShader = shaderCache.FindOrCreateShader("Shaders/basic_mesh.qsvert");
		VulkanShader* pFragmentShader = shaderCache.FindOrCreateShader("Shaders/basic_color.qsfrag");

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

			VulkanRenderablePerModelUBO perModelUbo = {};
			perModelUbo.model	= transformComponent.GetMatrix();
			perModelUbo.view	= cameraTransform.GetViewMatrix();
			perModelUbo.proj	= camera.GetProjectionMatrix();

			UniformBufferLocation transformBufferLocation;
			bufferCache.AllocateAndWriteUniformData(transformBufferLocation, 0, &perModelUbo, sizeof(VulkanRenderablePerModelUBO));

			for (const Mesh& mesh : pModel->meshes)
			{
				VulkanRenderable renderable = {};

				const IndexElement indexElement	= mesh.indexElement;
				const IndexFormat indexType		= indexElement.format;
				const uInt32 indexSize			= indexElement.FormatSize();

				renderable.meshBuffer		= bufferLocation;
				renderable.transformBuffer	= transformBufferLocation;
				renderable.indexStart		= mesh.indicesStartBytes / indexSize;
				renderable.indexCount		= mesh.indicesSizeBytes / indexSize; 
				renderable.vkIndexType		= indexType == INDEX_FORMAT_UINT16 ? VK_INDEX_TYPE_UINT16 : VK_INDEX_TYPE_UINT32;

				if (world.HasComponent<MaterialComponent>(entity))
				{
					MaterialComponent& materialComponent = world.Get<MaterialComponent>(entity);

					Array<VulkanShader*, 8> shaderList;

					const String& materialPath = materialComponent.materialPaths[0]; // @TODO mesh materialIdx
					Material* pMaterial = Engine::GetAssetManager().GetOrLoadAsset<Material>(materialPath); // @TODO: Cache
					
					if (!pMaterial)
					{
						LogError("Error prepairing Mesh [%s] for render: Invalid material path \"%s\"", mesh.name.Str(), materialPath.Str());
						return;
					}

					for (const String& shaderPath : pMaterial->shaderPaths)
					{
						VulkanShader* pVulkanShader = shaderCache.FindOrCreateShader(shaderPath);

						if (!pVulkanShader)
						{
							LogError("Error prepairing Mesh [%s] for render: Invalid shader path \"%s\"", mesh.name.Str(), shaderPath.Str());
							return;
						}
						
						shaderList.PushBack(pVulkanShader);
					}

					Array<VulkanAttachment, 2> attachments =
					{
						{ "Swapchain",		VULKAN_ATTACHMENT_TYPE_SWAPCHAIN,		VK_FORMAT_B8G8R8A8_UNORM },
						{ "Depth-Stencil",	VULKAN_ATTACHMENT_TYPE_DEPTH_STENCIL,	VK_FORMAT_D24_UNORM_S8_UINT }
					};

					VulkanGraphicsPipelineInfo pipelineInfo = pipelineCache.MakeGraphicsPipelineInfo(shaderList, attachments);

					pipelineInfo.vkFrontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
					pipelineInfo.vkCullMode = VK_CULL_MODE_NONE;

					renderable.pPipeline = pipelineCache.FindOrCreateGraphicsPipeline(pipelineInfo);

					if (renderable.pPipeline->descriptorSetLayouts[0]->setBindings.Size() > 1) //
					{
						// @TODO: All of this is bad VVVVVVVV

						uSize materialBufferSizeBytes = renderable.pPipeline->descriptorSetLayouts[0]->setBindings[1].sizeBytes;

						if (materialBufferSizeBytes % 64 != 0)
						{
							materialBufferSizeBytes += 64 - (materialBufferSizeBytes % 64);
						}

						ByteBuffer materialBuffer(materialBufferSizeBytes + 64);
						materialBuffer.Allocate(materialBufferSizeBytes + 64);

						for (auto& valuePair : pMaterial->shaderValues)
						{
							const String& paramName				= valuePair.key;
							const MaterialValue& paramValue		= valuePair.value;

							uSize paramOffsetBytes = 0;

							for (const VulkanShader* pVulkanShader : shaderList)
							{
								const Shader* pShaderAsset = pVulkanShader->pShaderAsset;
								for (const ShaderParam& param : pShaderAsset->params)
								{
									if (param.name == paramName)
									{
										MemCopy(materialBuffer.Data() + param.valueOffsetBytes, &paramValue.vec4uVal, param.valueSizeBytes);
										break;
									}
								}
							}
						}

						UniformBufferLocation materialBufferLocation;//                VVV fake set 1 for different buffer
						bufferCache.AllocateAndWriteUniformData(materialBufferLocation, 0, materialBuffer.Data(), materialBufferSizeBytes);
					
						renderable.materialBuffer = materialBufferLocation;
					}
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

			recorder.SetIndexBuffer(renderable.meshBuffer.pIndexBuffer->GetVulkanBuffer(),
				renderable.meshBuffer.indexEntry.offset, renderable.vkIndexType);

			VulkanBufferBind pVertexBufferBinds[] = 
			{ 
				{renderable.meshBuffer.pVertexBuffer->GetVulkanBuffer(), renderable.meshBuffer.vertexEntry.offset} 
			};

			recorder.SetVertexBuffers(pVertexBufferBinds, 1);

			Array<VulkanUniformBufferBind, 8> set0bufferBinds;
			Array<VulkanUniformBufferBind, 8> set1bufferBinds;

			VulkanUniformBufferBind uniformTransform = {};
			uniformTransform.binding	= 0;
			uniformTransform.pBuffer	= renderable.transformBuffer.pBuffer->GetVulkanBuffer();
			uniformTransform.offset		= renderable.transformBuffer.entry.offset;
			uniformTransform.range		= renderable.transformBuffer.entry.sizeBytes;

			set0bufferBinds.PushBack(uniformTransform);

			VulkanUniformBufferBind uniformMaterial = {};
			uniformMaterial.binding		= 1;
			uniformMaterial.pBuffer		= renderable.materialBuffer.pBuffer->GetVulkanBuffer();
			uniformMaterial.offset		= renderable.materialBuffer.entry.offset;
			uniformMaterial.range		= renderable.materialBuffer.entry.sizeBytes;

			set0bufferBinds.PushBack(uniformMaterial);
			//set0bufferBinds.PushBack(uniformMaterial);
			
			recorder.BindUniforms(renderable.pPipeline, 0, set0bufferBinds.Data(), set0bufferBinds.Size(), nullptr, 0);
			//recorder.BindUniforms(renderable.pPipeline, 1, set1bufferBinds.Data(), set1bufferBinds.Size(), nullptr, 0);

			recorder.DrawIndexed(1, renderable.indexCount, renderable.indexStart, 0);
		}
	}
}