#include "Vulkan/Renderers/VulkanSceneRenderer.h"

#include "Resource/Assets/Shader.h"
#include "Component/MeshComponent.h"
#include "Component/MaterialComponent.h"

#include "Engine.h"

// TEMP
#include "Resource/Assets/Image.h"

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

		VkSamplerCreateInfo defaultSamplerInfo{};
		defaultSamplerInfo.sType					= VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		defaultSamplerInfo.magFilter				= VK_FILTER_LINEAR;
		defaultSamplerInfo.minFilter				= VK_FILTER_LINEAR;
		defaultSamplerInfo.addressModeU				= VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		defaultSamplerInfo.addressModeV				= VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		defaultSamplerInfo.addressModeW				= VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		defaultSamplerInfo.anisotropyEnable			= VK_FALSE;
		defaultSamplerInfo.maxAnisotropy			= 1;
		defaultSamplerInfo.borderColor				= VK_BORDER_COLOR_INT_OPAQUE_BLACK;
		defaultSamplerInfo.unnormalizedCoordinates	= VK_FALSE;
		defaultSamplerInfo.compareEnable			= VK_FALSE;
		defaultSamplerInfo.compareOp				= VK_COMPARE_OP_ALWAYS;
		defaultSamplerInfo.mipmapMode				= VK_SAMPLER_MIPMAP_MODE_LINEAR;
		defaultSamplerInfo.mipLodBias				= 0.0f;
		defaultSamplerInfo.minLod					= 0.0f;
		defaultSamplerInfo.maxLod					= 0.0f;

		vkCreateSampler(mpDevice->vkDevice, &defaultSamplerInfo, VK_NULL_HANDLE, &mVkDefaultSampler);

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

			Array<VulkanAttachment, 2> attachments =
			{
				{ "Swapchain",		VULKAN_ATTACHMENT_TYPE_SWAPCHAIN,		VK_FORMAT_B8G8R8A8_UNORM },
				{ "Depth-Stencil",	VULKAN_ATTACHMENT_TYPE_DEPTH_STENCIL,	VK_FORMAT_D24_UNORM_S8_UINT }
			};

			Array<VkVertexInputAttributeDescription, 16> meshVertexAttributes;
			Array<VkVertexInputBindingDescription, 16> meshVertexBindings;

			uSize location = 0;
			for (const VertexStream& stream : pModel->vertexStreams)
			{
				if (!stream.pVertexBuffer)
				{
					continue;
				}

				VkVertexInputBindingDescription vertexBufferBinding = {};
				vertexBufferBinding.binding		= stream.streamIdx;
				vertexBufferBinding.stride		= stream.strideBytes;
				vertexBufferBinding.inputRate	= VK_VERTEX_INPUT_RATE_VERTEX;

				for (const VertexElement& element : stream.vertexElements)
				{
					VkFormat vkAttribFormat = VK_FORMAT_R32G32B32_SFLOAT;

					switch (element.format)
					{
						case VERTEX_FORMAT_FLOAT:				vkAttribFormat = VK_FORMAT_R32_SFLOAT; break;
						case VERTEX_FORMAT_FLOAT2:				vkAttribFormat = VK_FORMAT_R32G32_SFLOAT; break;
						case VERTEX_FORMAT_FLOAT3:				vkAttribFormat = VK_FORMAT_R32G32B32_SFLOAT; break;
						case VERTEX_FORMAT_FLOAT4:				vkAttribFormat = VK_FORMAT_R32G32B32A32_SFLOAT; break;
						case VERTEX_FORMAT_INT:					vkAttribFormat = VK_FORMAT_R32_SINT; break;
						case VERTEX_FORMAT_INT2:				vkAttribFormat = VK_FORMAT_R32G32_SINT; break;
						case VERTEX_FORMAT_INT3:				vkAttribFormat = VK_FORMAT_R32G32B32_SINT; break;
						case VERTEX_FORMAT_INT4:				vkAttribFormat = VK_FORMAT_R32G32B32A32_SINT; break;
						case VERTEX_FORMAT_UINT:				vkAttribFormat = VK_FORMAT_R32_UINT; break;
						case VERTEX_FORMAT_UINT2:				vkAttribFormat = VK_FORMAT_R32G32_UINT; break;
						case VERTEX_FORMAT_UINT3:				vkAttribFormat = VK_FORMAT_R32G32B32_UINT; break;
						case VERTEX_FORMAT_UINT4:				vkAttribFormat = VK_FORMAT_R32G32B32A32_UINT; break;
						case VERTEX_FORMAT_INT_2_10_10_10:		vkAttribFormat = VK_FORMAT_A2R10G10B10_SINT_PACK32; break;
						case VERTEX_FORMAT_UINT_2_10_10_10:		vkAttribFormat = VK_FORMAT_A2R10G10B10_UINT_PACK32; break;
						case VERTEX_FORMAT_FLOAT_10_11_11:		vkAttribFormat = VK_FORMAT_B10G11R11_UFLOAT_PACK32; break;
						case VERTEX_FORMAT_FLOAT_16_16_16_16:	vkAttribFormat = VK_FORMAT_R16G16B16A16_SFLOAT; break;
					}

					VkVertexInputAttributeDescription elementAttrib = {};
					elementAttrib.binding	= stream.streamIdx;
					elementAttrib.location	= location++;
					elementAttrib.format	= vkAttribFormat;
					elementAttrib.offset	= element.offsetBytes;

					meshVertexAttributes.PushBack(elementAttrib);
				}

				meshVertexBindings.PushBack(vertexBufferBinding);
			}

			const IndexElement indexElement = pModel->indexStream.indexElement;

			for (const Mesh& mesh : pModel->meshes)
			{
				VulkanRenderable renderable = {};

				const IndexFormat indexType = indexElement.format;

				renderable.meshBuffer		= bufferLocation;
				renderable.transformBuffer	= transformBufferLocation;
				renderable.indexStart		= mesh.indexStart;
				renderable.indexCount		= mesh.indexCount;
				renderable.vkIndexType		= indexType == INDEX_FORMAT_UINT16 ? VK_INDEX_TYPE_UINT16 : VK_INDEX_TYPE_UINT32;

				if (world.HasComponent<MaterialComponent>(entity))
				{
					MaterialComponent& materialComponent = world.Get<MaterialComponent>(entity);

					Array<VulkanShader*, 8> shaderList;

					uSize materialIdx = mesh.materialIdx;

					if (materialIdx >= materialComponent.materialPaths.Size())
					{
						materialIdx = 0; // @TODO: use default material instead
					}

					const String& materialPath = materialComponent.materialPaths[materialIdx];
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

					VulkanGraphicsPipelineInfo pipelineInfo = 
						pipelineCache.MakeGraphicsPipelineInfo(shaderList, attachments, meshVertexAttributes, meshVertexBindings);

					pipelineInfo.vkFrontFace	= VK_FRONT_FACE_COUNTER_CLOCKWISE;
					pipelineInfo.vkCullMode		= VK_CULL_MODE_NONE;

					renderable.pPipeline = pipelineCache.FindOrCreateGraphicsPipeline(pipelineInfo);

					// @TODO: Not the right place (or method, get from shader instead) VVV
					for (const VertexStream& stream : pModel->vertexStreams)
					{
						VulkanBufferBind vertexBufferBind = {};
						vertexBufferBind.pBuffer = bufferLocation.vertexBuffers[stream.streamIdx]->GetVulkanBuffer();
						vertexBufferBind.offset = bufferLocation.vertexEntries[stream.streamIdx].offset;

						// I dont like pushing 8 of these
						renderable.vertexBinds.PushBack(vertexBufferBind);
					}

					uSize bindingCount = renderable.pPipeline->descriptorSetLayouts[0]->setBindings.Size();
					for (uSize b = 1; b < bindingCount; b++) //
					{
						// @TODO: All of this is bad VVVVVVVV

						uSize materialBufferSizeBytes = renderable.pPipeline->descriptorSetLayouts[0]->setBindings[b].sizeBytes;
						bool isTextureBind = false;

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

							/// VVV absolutely horrendous
							if (paramValue.type == MATERIAL_VALUE_TEXTURE)
							{
								isTextureBind = true;

								const String& texturePath = paramValue.stringVal;

								auto& textureIt = mTextureCache.Find(texturePath);
								if (textureIt != mTextureCache.End())
								{
									VulkanUniformImageBind imageBind = {};
									imageBind.binding		= b;
									imageBind.pImageView	= textureIt->value;
									imageBind.vkLayout		= VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
									imageBind.vkSampler		= mVkDefaultSampler;

									renderable.imageBinds.PushBack(imageBind);
								}
								else
								{
									Image* pImage = Engine::GetAssetManager().GetOrLoadAsset<Image>(texturePath);

									VulkanImageInfo imageInfo = {};
									imageInfo.width			= pImage->width;
									imageInfo.height		= pImage->height;
									imageInfo.depth			= 1;
									imageInfo.layers		= 1;
									imageInfo.mips			= 1;
									imageInfo.vkFormat		= VK_FORMAT_R8G8B8A8_UNORM;
									imageInfo.vkImageType	= VK_IMAGE_TYPE_2D;
									imageInfo.vkUsageFlags	= VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;

									VulkanImage* pVulkanImage = mpGraphics->pResourceManager->CreateImage(mpDevice, imageInfo);

									VulkanBufferInfo bufferInfo = {};
									bufferInfo.sizeBytes			= pImage->pImageData->Size();
									bufferInfo.vkMemoryProperties	= VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
									bufferInfo.vkBufferUsage		= VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT;

									VulkanBuffer* pImageTransferBuffer = mpGraphics->pResourceManager->CreateBuffer(mpDevice, bufferInfo);

									VulkanBufferWriter imageBufferWriter(pImageTransferBuffer);
									void* pBufferData = imageBufferWriter.Map();
									MemCopy(pBufferData, pImage->pImageData->Data(), pImage->pImageData->Size());
									imageBufferWriter.Unmap();

									VulkanCommandPoolInfo terrainCommandPoolInfo = {};
									terrainCommandPoolInfo.queueFamilyIndex			= mpDevice->pPhysicalDevice->primaryQueueFamilyIndices.graphics;
									terrainCommandPoolInfo.vkCommandPoolCreateFlags	= VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

									VulkanCommandPool* pImmediateCommandPool = mpGraphics->pResourceManager->CreateCommandPool(mpDevice, terrainCommandPoolInfo);
									VulkanCommandBuffer* pImmediateCommandBuffer;
									mpGraphics->pResourceManager->CreateCommandBuffers(pImmediateCommandPool, 1, &pImmediateCommandBuffer);

									VkBufferImageCopy vkImageCopy = {};
									vkImageCopy.imageExtent.width				= imageInfo.width;
									vkImageCopy.imageExtent.height				= imageInfo.height;
									vkImageCopy.imageExtent.depth				= 1; //imageInfo.depth;
									vkImageCopy.imageOffset.x					= 0;
									vkImageCopy.imageOffset.y					= 0;
									vkImageCopy.imageOffset.z					= 0;
									vkImageCopy.imageSubresource.aspectMask		= VK_IMAGE_ASPECT_COLOR_BIT;
									vkImageCopy.imageSubresource.baseArrayLayer = 0;
									vkImageCopy.imageSubresource.layerCount		= 1;
									vkImageCopy.imageSubresource.mipLevel		= 0;
									vkImageCopy.bufferOffset					= 0;
									vkImageCopy.bufferRowLength					= 0;
									vkImageCopy.bufferImageHeight				= 0;

									VulkanCommandRecorder immediateRecorder(pImmediateCommandBuffer);
									immediateRecorder.BeginRecording();
									immediateRecorder.PipelineBarrierImageTransferDest(pVulkanImage);
									immediateRecorder.CopyBufferToImage(pImageTransferBuffer, pVulkanImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, { vkImageCopy });
									immediateRecorder.PipelineBarrierImageShaderRead(pVulkanImage);
									immediateRecorder.EndRecording();

									VulkanSubmission transferSubmition = {};
									transferSubmition.commandBuffers	= { pImmediateCommandBuffer };
									transferSubmition.waitSemaphores	= { };
									transferSubmition.waitStages		= { };
									transferSubmition.signalSemaphores	= { };

									VkFence vkTransferFence = 0;

									VkFenceCreateInfo fenceInfo = {};
									fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
									fenceInfo.flags = 0;
									fenceInfo.pNext = nullptr;

									vkCreateFence(mpDevice->vkDevice, &fenceInfo, VK_NULL_HANDLE, &vkTransferFence);

									mpGraphics->Submit(transferSubmition, mpDevice->queues.graphics, vkTransferFence);
									vkDeviceWaitIdle(mpDevice->vkDevice);

									vkWaitForFences(mpDevice->vkDevice, 1, &vkTransferFence, true, INT64_MAX);

									mpGraphics->pResourceManager->DestroyBuffer(pImageTransferBuffer);

									VulkanImageViewInfo viewInfo = {};
									viewInfo.pImage				= pVulkanImage;
									viewInfo.vkAspectFlags		= VK_IMAGE_ASPECT_COLOR_BIT;
									viewInfo.vkFormat			= VK_FORMAT_R8G8B8A8_UNORM;
									viewInfo.vkImageViewType	= VK_IMAGE_VIEW_TYPE_2D;
									viewInfo.layerCount			= 1;
									viewInfo.layerStart			= 0;
									viewInfo.mipCount			= 1;
									viewInfo.mipStart			= 0;

									VulkanImageView* pVulkanImageView = mpGraphics->pResourceManager->CreateImageView(mpDevice, viewInfo);

									VulkanUniformImageBind imageBind = {};
									imageBind.binding		= b;
									imageBind.pImageView	= pVulkanImageView;
									imageBind.vkLayout		= VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
									imageBind.vkSampler		= mVkDefaultSampler;

									renderable.imageBinds.PushBack(imageBind);

									mTextureCache.Put(texturePath, pVulkanImageView);
								}

								continue;
							}

							uSize paramOffsetBytes = 0;

							for (const VulkanShader* pVulkanShader : shaderList)
							{
								const Shader* pShaderAsset = pVulkanShader->pShaderAsset;
								for (const ShaderParam& param : pShaderAsset->params)
								{
									if (param.binding == b)
									{
										if (param.name == paramName)
										{
											MemCopy(materialBuffer.Data() + param.valueOffsetBytes, &paramValue.vec4uVal, param.valueSizeBytes);
											break;
										}
									}
								}
							}
						}

						if (!isTextureBind)
						{
							UniformBufferLocation materialBufferLocation;//                VVV fake set 1 for different buffer
							bufferCache.AllocateAndWriteUniformData(materialBufferLocation, 0, materialBuffer.Data(), materialBufferSizeBytes);
					
							renderable.materialBuffer = materialBufferLocation;
						}
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

			//array<VulkanBufferBind, 8> vertexBufferBinds;

			//VulkanBufferBind pVertexBufferBinds[] = 
			//{ 
			//	{renderable.meshBuffer.pVertexBuffer->GetVulkanBuffer(), renderable.meshBuffer.vertexEntry.offset} 
			//};

			recorder.SetVertexBuffers(renderable.vertexBinds.Data(), renderable.vertexBinds.Size());

			Array<VulkanUniformBufferBind, 8> set0bufferBinds;
			Array<VulkanUniformBufferBind, 8> set1bufferBinds;

			VulkanUniformBufferBind uniformTransform = {}; // Move into the Renderable
			uniformTransform.binding	= 0;
			uniformTransform.pBuffer	= renderable.transformBuffer.pBuffer->GetVulkanBuffer();
			uniformTransform.offset		= renderable.transformBuffer.entry.offset;
			uniformTransform.range		= renderable.transformBuffer.entry.sizeBytes;

			set0bufferBinds.PushBack(uniformTransform);

			if (renderable.materialBuffer.pBuffer)
			{
				VulkanUniformBufferBind uniformMaterial = {}; // Move into the Renderable
				uniformMaterial.binding		= 1;
				uniformMaterial.pBuffer		= renderable.materialBuffer.pBuffer->GetVulkanBuffer();
				uniformMaterial.offset		= renderable.materialBuffer.entry.offset;
				uniformMaterial.range		= renderable.materialBuffer.entry.sizeBytes;

				set0bufferBinds.PushBack(uniformMaterial);
			}

			recorder.BindUniforms(renderable.pPipeline, 0, set0bufferBinds.Data(), set0bufferBinds.Size(), 
				renderable.imageBinds.Data(), renderable.imageBinds.Size());
			//recorder.BindUniforms(renderable.pPipeline, 1, set1bufferBinds.Data(), set1bufferBinds.Size(), nullptr, 0);

			recorder.DrawIndexed(1, renderable.indexCount, renderable.indexStart, 0);
		}
	}
}