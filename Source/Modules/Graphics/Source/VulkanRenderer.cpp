#include "Vulkan/VulkanRenderer.h"

#include "Log.h"
#include "Engine.h"
#include "Vulkan/VulkanGraphics.h"
#include "Vulkan/Primatives/VulkanPipeline.h"
#include "Vulkan/VulkanCommandRecorder.h"
#include "Vulkan/VulkanSwapchainTimer.h"
#include "Vulkan/VulkanBufferWriter.h"
#include "Vulkan/VulkanBufferCache.h"

#include "Vulkan/VulkanMultiBuffer.h"

#include "Component/MeshComponent.h"
#include "Component/TransformComponent.h"

// TEMP
#include "TerrainRenderer.h"

namespace Quartz
{
	VulkanImageView* pPerlinImageView = nullptr;
	VkSampler perlinSampler;

	void VulkanRenderer::Initialize(VulkanGraphics* pGraphics)
	{
		VulkanResourceManager*	pResources	= pGraphics->pResourceManager;
		VulkanDevice*			pDevice		= pGraphics->pPrimaryDevice;

		mpGraphics		= pGraphics;
		mPipelineCache	= VulkanPipelineCache(pDevice, pResources);
		mShaderCache	= VulkanShaderCache(pDevice, pResources);
		mpSwapchain		= pResources->CreateSwapchain(pGraphics->pPrimaryDevice, *pGraphics->pSurface, 3);
		mSwapTimer		= VulkanSwapchainTimer(mpSwapchain);

		VulkanRenderSettings renderSettings = {};
		renderSettings.useUniqueMeshBuffers				= false;
		renderSettings.useUniqueMeshStagingBuffers		= false;
		renderSettings.useUniqueUniformBuffers			= false;
		renderSettings.useUniqueUniformStagingBuffers	= false;
		renderSettings.vertexBufferSizeMb				= 32;
		renderSettings.indexBufferSizeMb				= 16;
		renderSettings.perInstanceBufferSizeMb			= 16;
		renderSettings.perModelBufferSizeMb				= 32;
		renderSettings.globalBufferSizeBytes			= 128;
		renderSettings.uniquePerInstanceBufferSizeBytes	= 128; //
		renderSettings.uniquePerModelBufferSizeBytes	= 128; //
		renderSettings.useInstancing					= false;
		renderSettings.useMeshStaging					= true;
		renderSettings.useUniformStaging				= true;
		renderSettings.useDrawIndirect					= false;

		mBufferCache.Initialize(pDevice, pResources, renderSettings);

		VulkanShader* pVertexShader = mShaderCache.FindOrCreateShader("Shaders/default.vert", VK_SHADER_STAGE_VERTEX_BIT);
		VulkanShader* pFragmentShader = mShaderCache.FindOrCreateShader("Shaders/default.frag", VK_SHADER_STAGE_FRAGMENT_BIT);

		Array<VulkanAttachment> attachments =
		{
			{ "Swapchain",		VULKAN_ATTACHMENT_TYPE_SWAPCHAIN,		VK_FORMAT_B8G8R8A8_UNORM },
			{ "Depth-Stencil",	VULKAN_ATTACHMENT_TYPE_DEPTH_STENCIL,	VK_FORMAT_D24_UNORM_S8_UINT }
		};

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

		mpDefaultPipeline = mPipelineCache.FindOrCreateGraphicsPipeline(
			{ pVertexShader, pFragmentShader },
			attachments, vertexAttributes, vertexBindings
		);

		if (!mpDefaultPipeline)
		{
			LogFatal("Failed to create Pipeline!");
		}

		for (uSize i = 0; i < 3; i++)
		{
			VulkanImageInfo depthImageInfo = {};
			depthImageInfo.vkFormat		= VK_FORMAT_D24_UNORM_S8_UINT;
			depthImageInfo.vkImageType	= VK_IMAGE_TYPE_2D;
			depthImageInfo.vkUsageFlags = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
			depthImageInfo.width		= pGraphics->pSurface->width;
			depthImageInfo.height		= pGraphics->pSurface->height;
			depthImageInfo.depth		= 1;
			depthImageInfo.layers		= 1;
			depthImageInfo.mips			= 1;

			mDepthImages[i] = pResources->CreateImage(pDevice, depthImageInfo);
			LogInfo("Image %p", mDepthImages[i]);

			VulkanImageViewInfo depthImageViewInfo = {};
			depthImageViewInfo.pImage			= mDepthImages[i];
			depthImageViewInfo.vkFormat			= VK_FORMAT_D24_UNORM_S8_UINT;
			depthImageViewInfo.vkImageViewType	= VK_IMAGE_VIEW_TYPE_2D;
			depthImageViewInfo.vkAspectFlags	= VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
			depthImageViewInfo.layerStart		= 0;
			depthImageViewInfo.layerCount		= 1;
			depthImageViewInfo.mipStart			= 0;
			depthImageViewInfo.mipCount			= 1;

			mDepthImageViews[i] = pResources->CreateImageView(pDevice, depthImageViewInfo);
			LogInfo("View %p", mDepthImageViews[i]);
		}

		VulkanCommandPoolInfo renderPoolInfo = {};
		renderPoolInfo.queueFamilyIndex			= pGraphics->pPrimaryDevice->pPhysicalDevice-> primaryQueueFamilyIndices.graphics;
		renderPoolInfo.vkCommandPoolCreateFlags	= VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

		VulkanCommandPool* pRenderPool = pResources->CreateCommandPool(pGraphics->pPrimaryDevice, renderPoolInfo);

		pResources->CreateCommandBuffers(pRenderPool, 3, mCommandBuffers);

		///// TEMP /////

		uSize perlinResolution = 1024;

		TerrainRenderer terrainRenderer;
		Array<float> perlin = terrainRenderer.CreatePerlinNoiseTexture(perlinResolution, 1234, { 1.0f, 0.5f, 0.25f, 0.125f, 0.05f, 0.01f, 0.001f, 0.0001f });

		VulkanImageInfo perlinImageInfo = {};
		perlinImageInfo.vkImageType		= VK_IMAGE_TYPE_2D;
		perlinImageInfo.vkFormat		= VK_FORMAT_R32_SFLOAT;
		perlinImageInfo.vkUsageFlags	= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
		perlinImageInfo.width			= perlinResolution;
		perlinImageInfo.height			= perlinResolution;
		perlinImageInfo.depth			= 1;
		perlinImageInfo.layers			= 1;
		perlinImageInfo.mips			= 1;

		VulkanImage* pPerlinImage = pResources->CreateImage(pDevice, perlinImageInfo);

		VulkanImageViewInfo perlinImageViewInfo = {};
		perlinImageViewInfo.pImage				= pPerlinImage;
		perlinImageViewInfo.vkImageViewType		= VK_IMAGE_VIEW_TYPE_2D;
		perlinImageViewInfo.vkAspectFlags		= VK_IMAGE_ASPECT_COLOR_BIT;
		perlinImageViewInfo.vkFormat			= VK_FORMAT_R32_SFLOAT;
		perlinImageViewInfo.mipStart			= 0;
		perlinImageViewInfo.mipCount			= 1;
		perlinImageViewInfo.layerStart			= 0;
		perlinImageViewInfo.layerCount			= 1;

		pPerlinImageView = pResources->CreateImageView(pDevice, perlinImageViewInfo);

		VulkanBufferInfo perlinImageBufferInfo = {};
		perlinImageBufferInfo.sizeBytes				= perlinResolution * perlinResolution * sizeof(float);
		perlinImageBufferInfo.vkBufferUsage			= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
		perlinImageBufferInfo.vkMemoryProperties	= VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

		VulkanBuffer* pPerlinImageBuffer = pResources->CreateBuffer(pDevice, perlinImageBufferInfo);

		VulkanBufferWriter perlinWriter(pPerlinImageBuffer);
		float* pPerlinData = perlinWriter.Map<float>();

		memcpy_s(pPerlinData, perlin.Size() * sizeof(float), perlin.Data(), perlin.Size() * sizeof(float));

		VulkanCommandBuffer* pImmediateBuffer;
		pResources->CreateCommandBuffers(pRenderPool, 1, &pImmediateBuffer);

		VulkanCommandRecorder immediateRecorder(pImmediateBuffer);

		immediateRecorder.BeginRecording();

		VkImageMemoryBarrier vkPerlinMemoryBarrier = {};
		vkPerlinMemoryBarrier.sType								= VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		vkPerlinMemoryBarrier.srcAccessMask						= 0;
		vkPerlinMemoryBarrier.dstAccessMask						= VK_ACCESS_TRANSFER_WRITE_BIT;
		vkPerlinMemoryBarrier.oldLayout							= VK_IMAGE_LAYOUT_UNDEFINED;
		vkPerlinMemoryBarrier.newLayout							= VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		vkPerlinMemoryBarrier.image								= pPerlinImage->vkImage;
		vkPerlinMemoryBarrier.subresourceRange.aspectMask		= VK_IMAGE_ASPECT_COLOR_BIT;
		vkPerlinMemoryBarrier.subresourceRange.baseMipLevel		= 0;
		vkPerlinMemoryBarrier.subresourceRange.levelCount		= 1;
		vkPerlinMemoryBarrier.subresourceRange.baseArrayLayer	= 0;
		vkPerlinMemoryBarrier.subresourceRange.layerCount		= 1;

		VulkanPipelineBarrierInfo perlinBarrierInfo = {};
		perlinBarrierInfo.srcStage					= VK_PIPELINE_STAGE_TRANSFER_BIT;
		perlinBarrierInfo.dstStage					= VK_PIPELINE_STAGE_TRANSFER_BIT;
		perlinBarrierInfo.dependencyFlags			= 0;
		perlinBarrierInfo.memoryBarrierCount		= 0;
		perlinBarrierInfo.pMemoryBarriers			= nullptr;
		perlinBarrierInfo.bufferMemoryBarrierCount	= 0;
		perlinBarrierInfo.pBufferMemoryBarriers		= nullptr;
		perlinBarrierInfo.imageMemoryBarrierCount	= 1;
		perlinBarrierInfo.pImageMemoryBarriers		= &vkPerlinMemoryBarrier;

		immediateRecorder.PipelineBarrier(perlinBarrierInfo);

		VkBufferImageCopy vkPerlinImageCopy = {};
		vkPerlinImageCopy.bufferOffset						= 0;
		vkPerlinImageCopy.bufferRowLength					= 0;
		vkPerlinImageCopy.bufferImageHeight					= 0;
		vkPerlinImageCopy.imageSubresource.aspectMask		= VK_IMAGE_ASPECT_COLOR_BIT;
		vkPerlinImageCopy.imageSubresource.baseArrayLayer	= 0;
		vkPerlinImageCopy.imageSubresource.layerCount		= 1;
		vkPerlinImageCopy.imageSubresource.mipLevel			= 0;
		vkPerlinImageCopy.imageOffset.x						= 0;
		vkPerlinImageCopy.imageOffset.y						= 0;
		vkPerlinImageCopy.imageOffset.z						= 0;
		vkPerlinImageCopy.imageExtent.width					= perlinResolution;
		vkPerlinImageCopy.imageExtent.height				= perlinResolution;
		vkPerlinImageCopy.imageExtent.depth					= 1;

		immediateRecorder.CopyBufferToImage(pPerlinImageBuffer, pPerlinImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, { vkPerlinImageCopy });

		VkImageMemoryBarrier vkPerlinMemoryBarrier2 = {};
		vkPerlinMemoryBarrier2.sType							= VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		vkPerlinMemoryBarrier2.srcAccessMask					= 0;
		vkPerlinMemoryBarrier2.dstAccessMask					= VK_ACCESS_SHADER_READ_BIT;
		vkPerlinMemoryBarrier2.oldLayout						= VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		vkPerlinMemoryBarrier2.newLayout						= VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		vkPerlinMemoryBarrier2.image							= pPerlinImage->vkImage;
		vkPerlinMemoryBarrier2.subresourceRange.aspectMask		= VK_IMAGE_ASPECT_COLOR_BIT;
		vkPerlinMemoryBarrier2.subresourceRange.baseMipLevel	= 0;
		vkPerlinMemoryBarrier2.subresourceRange.levelCount		= 1;
		vkPerlinMemoryBarrier2.subresourceRange.baseArrayLayer	= 0;
		vkPerlinMemoryBarrier2.subresourceRange.layerCount		= 1;

		VulkanPipelineBarrierInfo perlinBarrierInfo2 = {};
		perlinBarrierInfo2.srcStage					= VK_PIPELINE_STAGE_VERTEX_SHADER_BIT;
		perlinBarrierInfo2.dstStage					= VK_PIPELINE_STAGE_VERTEX_SHADER_BIT;
		perlinBarrierInfo2.dependencyFlags			= 0;
		perlinBarrierInfo2.memoryBarrierCount		= 0;
		perlinBarrierInfo2.pMemoryBarriers			= nullptr;
		perlinBarrierInfo2.bufferMemoryBarrierCount	= 0;
		perlinBarrierInfo2.pBufferMemoryBarriers	= nullptr;
		perlinBarrierInfo2.imageMemoryBarrierCount	= 1;
		perlinBarrierInfo2.pImageMemoryBarriers		= &vkPerlinMemoryBarrier2;

		immediateRecorder.PipelineBarrier(perlinBarrierInfo2);

		immediateRecorder.EndRecording();

		VulkanSubmission renderSubmition	= {};
		renderSubmition.commandBuffers		= { pImmediateBuffer };
		renderSubmition.waitSemaphores		= {};
		renderSubmition.waitStages			= {};
		renderSubmition.signalSemaphores	= {};

		mpGraphics->Submit(renderSubmition, mpGraphics->pPrimaryDevice->queues.graphics, VK_NULL_HANDLE);

		VkSamplerCreateInfo perlinSamplerInfo{};
		perlinSamplerInfo.sType				= VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		perlinSamplerInfo.magFilter			= VK_FILTER_LINEAR;
		perlinSamplerInfo.minFilter			= VK_FILTER_LINEAR;
		perlinSamplerInfo.addressModeU		= VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
		perlinSamplerInfo.addressModeV		= VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
		perlinSamplerInfo.addressModeW		= VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
		perlinSamplerInfo.anisotropyEnable	= VK_FALSE;
		perlinSamplerInfo.maxAnisotropy		= 1;
		perlinSamplerInfo.borderColor		= VK_BORDER_COLOR_INT_OPAQUE_BLACK;
		perlinSamplerInfo.unnormalizedCoordinates = VK_FALSE;
		perlinSamplerInfo.compareEnable		= VK_FALSE;
		perlinSamplerInfo.compareOp			= VK_COMPARE_OP_ALWAYS;
		perlinSamplerInfo.mipmapMode		= VK_SAMPLER_MIPMAP_MODE_LINEAR;
		perlinSamplerInfo.mipLodBias		= 0.0f;
		perlinSamplerInfo.minLod			= 0.0f;
		perlinSamplerInfo.maxLod			= 0.0f;

		vkCreateSampler(mpGraphics->pPrimaryDevice->vkDevice, &perlinSamplerInfo, VK_NULL_HANDLE, &perlinSampler);
		
		////////////////
	}

	void VulkanRenderer::SetCamera(Entity cameraEntity)
	{
		if (!Engine::GetWorld().HasComponent<CameraComponent>(cameraEntity) ||
			!Engine::GetWorld().HasComponent<TransformComponent>(cameraEntity))
		{
			return; // Error
		}

		mCameraEntity				= cameraEntity;
		mpCameraComponent			= &Engine::GetWorld().Get<CameraComponent>(cameraEntity);
		mpCameraTransformComponent	= &Engine::GetWorld().Get<TransformComponent>(cameraEntity);
	}

	struct PerModelUBO
	{
		Mat4f model;
		Mat4f view;
		Mat4f proj;
	};

	void VulkanRenderer::UpdateAll(EntityWorld* pWorld)
	{
		auto& renderableView = pWorld->CreateView<MeshComponent, TransformComponent>();

		mRenderables.Clear();

		mBufferCache.ResetPerModelBuffers();

		for (Entity& entity : renderableView)
		{
			MeshComponent& meshComponent			= pWorld->Get<MeshComponent>(entity);
			TransformComponent& transformComponent	= pWorld->Get<TransformComponent>(entity);

			VulkanRenderable renderable = {};

			bool vertexDataFound;
			mBufferCache.FillRenderableVertexData(renderable, meshComponent.modelURIHash, &meshComponent.modelData, vertexDataFound);

			PerModelUBO perModelUbo = {};
			perModelUbo.model	= transformComponent.GetMatrix();
			perModelUbo.view	= mpCameraTransformComponent->GetViewMatrix();
			perModelUbo.proj	= mpCameraComponent->GetProjectionMatrix(1280, 720);

			mBufferCache.FillRenderablePerModelData(renderable, 0, &perModelUbo, sizeof(PerModelUBO));

			if (pWorld->HasComponent<MaterialComponent>(entity))
			{
				MaterialComponent& materialComponent = pWorld->Get<MaterialComponent>(entity);

				VulkanShader* pVertexShader		= mShaderCache.FindOrCreateShader(materialComponent.vertexURI, VK_SHADER_STAGE_VERTEX_BIT);
				VulkanShader* pFragmentShader	= mShaderCache.FindOrCreateShader(materialComponent.fragmentURI, VK_SHADER_STAGE_FRAGMENT_BIT);

				VulkanGraphicsPipeline* pPipeline = mPipelineCache.FindOrCreateGraphicsPipeline(
					{ pVertexShader, pFragmentShader },
					mpDefaultPipeline->pipelineInfo.attachments,
					mpDefaultPipeline->pipelineInfo.vertexAttributes,
					mpDefaultPipeline->pipelineInfo.vertexBindings
				);

				renderable.pPipeline = pPipeline;
			}
			else
			{
				renderable.pPipeline = mpDefaultPipeline;
			}

			if (pWorld->HasComponent<TerrainComponent>(entity))
			{
				renderable.isTerrain = true;
			}

			mRenderables.PushBack(renderable);
		}
	}

	void VulkanRenderer::WriteCommandBuffer(VulkanCommandRecorder* pRecorder)
	{
		for (VulkanRenderable& renderable : mRenderables)
		{
			pRecorder->SetGraphicsPipeline(renderable.pPipeline);

			pRecorder->SetIndexBuffer(renderable.meshLocation.pIndexBuffer->GetVulkanBuffer(),
				renderable.meshLocation.indexEntry.offset, VK_INDEX_TYPE_UINT16);

			VulkanBufferBind pVertexBufferBinds[] = 
			{ 
				{renderable.meshLocation.pVertexBuffer->GetVulkanBuffer(), renderable.meshLocation.vertexEntry.offset} 
			};

			pRecorder->SetVertexBuffers(pVertexBufferBinds, 1);

			VulkanUniformBufferBind binding = {};
			binding.binding = 0;
			binding.pBuffer = renderable.perModelLocation.pPerModelBuffer->GetVulkanBuffer();
			binding.offset	= renderable.perModelLocation.perModelEntry.offset;
			binding.range	= renderable.perModelLocation.perModelEntry.sizeBytes;

			VulkanUniformBufferBind pBufferBinds[] = { binding };

			if (renderable.isTerrain)
			{
				VulkanUniformImageBind imageBinding = {};
				imageBinding.binding	= 1;
				imageBinding.vkSampler	= perlinSampler;
				imageBinding.pImageView = pPerlinImageView;
				imageBinding.vkLayout	= VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

				VulkanUniformImageBind pImageBinds[] = { imageBinding };

				pRecorder->BindUniforms(renderable.pPipeline, 0, pBufferBinds, 1, pImageBinds, 1);
			}
			else
			{
				pRecorder->BindUniforms(renderable.pPipeline, 0, pBufferBinds, 1, nullptr, 0);
			}

			pRecorder->DrawIndexed(1, renderable.indexCount, 0); //renderable.meshLocation.indexEntry.offset / sizeof(uInt16)
		}
	}

	void VulkanRenderer::RenderScene(EntityWorld* pWorld)
	{
		mSwapTimer.AdvanceFrame();

		uInt32 resourceIdx = mSwapTimer.GetFrameIndex();
		VulkanCommandBuffer* pCommandBuffer = mCommandBuffers[resourceIdx];
		VulkanCommandRecorder recorder(pCommandBuffer);

		VulkanSubmission renderSubmition	= {};
		renderSubmition.commandBuffers		= { mCommandBuffers[resourceIdx] };
		renderSubmition.waitSemaphores		= { mSwapTimer.GetCurrentAcquiredSemaphore() };
		renderSubmition.waitStages			= { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
		renderSubmition.signalSemaphores	= { mSwapTimer.GetCurrentCompleteSemaphore() };

		UpdateAll(pWorld);

		recorder.Reset();
		
		recorder.BeginRecording();

		VkViewport vkViewport = {};
		vkViewport.x		= 0;
		vkViewport.y		= mpGraphics->pSurface->height;
		vkViewport.width	= mpGraphics->pSurface->width;
		vkViewport.height	= -(float)mpGraphics->pSurface->height;
		vkViewport.minDepth = 0.0f;
		vkViewport.maxDepth = 1.0f;

		VkRect2D vkScissor = {};
		vkScissor.offset.x		= 0;
		vkScissor.offset.y		= 0;
		vkScissor.extent.width	= mpGraphics->pSurface->width;
		vkScissor.extent.height = mpGraphics->pSurface->height;

		recorder.SetViewport(vkViewport, vkScissor);

		mBufferCache.RecordTransfers(&recorder);

		recorder.PipelineBarrierSwapchainImageBegin(mpSwapchain->images[resourceIdx]);

		VkImageMemoryBarrier vkDepthImageMemoryBarrier = {};
		vkDepthImageMemoryBarrier.sType								= VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		vkDepthImageMemoryBarrier.srcAccessMask						= 0;
		vkDepthImageMemoryBarrier.dstAccessMask						= VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
		vkDepthImageMemoryBarrier.oldLayout							= VK_IMAGE_LAYOUT_UNDEFINED;
		vkDepthImageMemoryBarrier.newLayout							= VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		vkDepthImageMemoryBarrier.image								= mDepthImages[resourceIdx]->vkImage;
		vkDepthImageMemoryBarrier.subresourceRange.aspectMask		= VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
		vkDepthImageMemoryBarrier.subresourceRange.baseMipLevel		= 0;
		vkDepthImageMemoryBarrier.subresourceRange.levelCount		= 1;
		vkDepthImageMemoryBarrier.subresourceRange.baseArrayLayer	= 0;
		vkDepthImageMemoryBarrier.subresourceRange.layerCount		= 1;

		VulkanPipelineBarrierInfo barrierInfo = {};
		barrierInfo.srcStage					= VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
		barrierInfo.dstStage					= VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
		barrierInfo.dependencyFlags				= 0;
		barrierInfo.memoryBarrierCount			= 0;
		barrierInfo.pMemoryBarriers				= nullptr;
		barrierInfo.bufferMemoryBarrierCount	= 0;
		barrierInfo.pBufferMemoryBarriers		= nullptr;
		barrierInfo.imageMemoryBarrierCount		= 1;
		barrierInfo.pImageMemoryBarriers		= &vkDepthImageMemoryBarrier;

		recorder.PipelineBarrier(barrierInfo);

		VulkanRenderingAttachmentInfo swapchainRenderingAttachmentInfo = {};
		swapchainRenderingAttachmentInfo.pImageView		= mpSwapchain->imageViews[resourceIdx];
		swapchainRenderingAttachmentInfo.imageLayout	= VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		swapchainRenderingAttachmentInfo.loadOp			= VK_ATTACHMENT_LOAD_OP_CLEAR;
		swapchainRenderingAttachmentInfo.storeOp		= VK_ATTACHMENT_STORE_OP_STORE;
		swapchainRenderingAttachmentInfo.clearValue		= { 0.02f, 0.05f, 0.05f, 1.0f };

		VulkanRenderingAttachmentInfo depthRenderingAttachmentInfo = {};
		depthRenderingAttachmentInfo.pImageView		= mDepthImageViews[resourceIdx];
		depthRenderingAttachmentInfo.imageLayout	= VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		depthRenderingAttachmentInfo.loadOp			= VK_ATTACHMENT_LOAD_OP_CLEAR;
		depthRenderingAttachmentInfo.storeOp		= VK_ATTACHMENT_STORE_OP_STORE;
		depthRenderingAttachmentInfo.clearValue		= { 1.0f, 0 };

		VulkanRenderingAttachmentInfo pColorAttachmentInfos[] = { swapchainRenderingAttachmentInfo };

		VulkanRenderingBeginInfo renderingBeginInfo = {};
		renderingBeginInfo.pColorAttachments	= pColorAttachmentInfos;
		renderingBeginInfo.colorAttachmentCount	= 1;
		renderingBeginInfo.pDepthAttachment		= &depthRenderingAttachmentInfo;
		renderingBeginInfo.pStencilAttachment	= nullptr;
		renderingBeginInfo.renderArea			= { { 0, 0 }, { mpGraphics->pSurface->width, mpGraphics->pSurface->height } };

		recorder.BeginRendering(renderingBeginInfo);

		WriteCommandBuffer(&recorder);

		recorder.EndRendering();

		recorder.PipelineBarrierSwapchainImageEnd(mpSwapchain->images[resourceIdx]);

		recorder.EndRecording();

		mpGraphics->Submit(renderSubmition, mpGraphics->pPrimaryDevice->queues.graphics, mSwapTimer.GetCurrentFence());

		mSwapTimer.Present();
	}

	void VulkanRenderer::RenderUpdate(Runtime* pRuntime, double delta)
	{
		RenderScene(&Engine::GetWorld());
	}

	void VulkanRenderer::Register(Runtime* pRuntime)
	{
		pRuntime->RegisterOnUpdate(&VulkanRenderer::RenderUpdate, this);
	}
}