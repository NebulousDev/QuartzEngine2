#include "Vulkan/VulkanRenderer.h"

#include "Log.h"
#include "Vulkan/VulkanGraphics.h"
#include "Vulkan/Primatives/VulkanPipeline.h"
#include "Vulkan/VulkanCommandRecorder.h"
#include "Vulkan/VulkanSwapchainTimer.h"
#include "Vulkan/VulkanBufferWriter.h"
#include "Vulkan/VulkanBufferCache.h"

#include "Vulkan/VulkanMultiBuffer.h"

#include "Component/MeshComponent.h"
#include "Component/TransformComponent.h"

namespace Quartz
{
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
		}

		VulkanCommandPoolInfo renderPoolInfo = {};
		renderPoolInfo.queueFamilyIndex			= pGraphics->pPrimaryDevice->pPhysicalDevice-> primaryQueueFamilyIndices.graphics;
		renderPoolInfo.vkCommandPoolCreateFlags	= VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

		VulkanCommandPool* pRenderPool = pResources->CreateCommandPool(pGraphics->pPrimaryDevice, renderPoolInfo);

		pResources->CreateCommandBuffers(pRenderPool, 3, mCommandBuffers);
	}

	struct PerModelUBO
	{
		Mat4f model;
		Mat4f view;
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
			perModelUbo.view	= Mat4f().SetTranslation({ 0.0f, 0.0f, -1.0f }) * Mat4f().SetPerspective(ToRadians(70.0f), (float)1280 / (float)720, 0.001f, 1000.0f);

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

			VulkanUniformBind binding = {};
			binding.binding = 0;
			binding.pBuffer = renderable.perModelLocation.pPerModelBuffer->GetVulkanBuffer();
			binding.offset	= renderable.perModelLocation.perModelEntry.offset;
			binding.range	= renderable.perModelLocation.perModelEntry.sizeBytes;

			VulkanUniformBind pUniformBinds[] = { binding };

			pRecorder->BindUniforms(renderable.pPipeline, 0, pUniformBinds, 1);
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

		vkResetCommandBuffer(pCommandBuffer->vkCommandBuffer, 0);
		
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
		RenderScene(mpGraphics->pEntityWorld);
	}

	void VulkanRenderer::Register(Runtime* pRuntime)
	{
		pRuntime->RegisterOnUpdate(&VulkanRenderer::RenderUpdate, this);
	}
}