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

namespace Quartz
{
	VulkanImageView* pPerlinImageView = nullptr;
	VkSampler perlinSampler;

	void VulkanRenderer::Initialize(VulkanGraphics& graphics, void* pWindowHandle, uSize maxInFlightCount)
	{
		VulkanResourceManager*	pResources	= graphics.pResourceManager;
		VulkanDevice*			pDevice		= graphics.pPrimaryDevice;

		mpGraphics		= &graphics;
		mPipelineCache	= VulkanPipelineCache(pDevice, pResources);
		mShaderCache	= VulkanShaderCache(pDevice, pResources);
		mpSwapchain		= pResources->CreateSwapchain(pDevice, *graphics.pSurface, maxInFlightCount);
		mSwapTimer		= VulkanSwapchainTimer(mpSwapchain);

		assert(maxInFlightCount < VULKAN_GRAPHICS_MAX_IN_FLIGHT);
		mMaxInFlightCount = maxInFlightCount;

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

		for (uSize i = 0; i < maxInFlightCount; i++)
		{
			VulkanImageInfo depthImageInfo = {};
			depthImageInfo.vkFormat		= VK_FORMAT_D24_UNORM_S8_UINT;
			depthImageInfo.vkImageType	= VK_IMAGE_TYPE_2D;
			depthImageInfo.vkUsageFlags = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
			depthImageInfo.width		= graphics.pSurface->width;
			depthImageInfo.height		= graphics.pSurface->height;
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
		renderPoolInfo.queueFamilyIndex			= pDevice->pPhysicalDevice-> primaryQueueFamilyIndices.graphics;
		renderPoolInfo.vkCommandPoolCreateFlags	= VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

		VulkanCommandPool* pRenderPool = pResources->CreateCommandPool(graphics.pPrimaryDevice, renderPoolInfo);

		pResources->CreateCommandBuffers(pRenderPool, maxInFlightCount, mCommandBuffers);

		mTerrainRenderer.Initialize(graphics, mShaderCache, mPipelineCache, maxInFlightCount);

		AtmosphereSun sun0 = {};
		sun0.sunDir = { 0.0f, -0.2f, -1.0f };
		sun0.sunIntensity = 1.0f;

		AtmosphereSun sun1 = {};
		sun1.sunDir = { 0.0f, 0.0f, 0.0f };
		sun1.sunIntensity = 0.0f;

		AtmosphereValues atmosphere = {};
		atmosphere.rayleighScattering	= { 5.802f, 13.558f, 33.1f };
		atmosphere.rayleighAbsorbtion	= 0;
		atmosphere.mieScattering		= 3.996;
		atmosphere.mieAbsorbtion		= 4.40;
		atmosphere.ozoneScattering		= 0;
		atmosphere.ozoneAbsorbtion		= { 0.650f, 1.881f, 0.085f };
		atmosphere.suns[0]				= sun0;
		atmosphere.suns[1]				= sun1;

		SkyRenderSettings settings = {};
		settings.transmittanceLUTSize = { 256, 64 };
		settings.scatterLUTSize = { 32, 32 };
		settings.viewLUTSize = { 200, 200 };

		mSceneRenderer.Initialize(graphics, mShaderCache, mPipelineCache, maxInFlightCount);
		mSkyRenderer.Initialize(graphics, atmosphere, settings, mShaderCache, mPipelineCache, maxInFlightCount);
		mImGuiRenderer.Initialize(graphics, pWindowHandle, *mSwapTimer.GetSwapchain());
	}

	void VulkanRenderer::SetCamera(Entity cameraEntity)
	{
		if (!Engine::GetWorld().HasComponent<CameraComponent>(cameraEntity) ||
			!Engine::GetWorld().HasComponent<TransformComponent>(cameraEntity))
		{
			return; // Error
		}

		mCameraEntity = cameraEntity;
	}

	void VulkanRenderer::UpdateAll(EntityWorld& world, uSize frameIdx)
	{
		TransformComponent& cameraTransformComponent = world.Get<TransformComponent>(mCameraEntity);
		CameraComponent& cameraComponent = world.Get<CameraComponent>(mCameraEntity);

		mSceneRenderer.Update(world, mBufferCache, mShaderCache, mPipelineCache, cameraComponent, cameraTransformComponent, frameIdx);

		Vec2f centerPos = { cameraTransformComponent.position.x, cameraTransformComponent.position.z };
		mTerrainRenderer.Update(centerPos, cameraComponent, cameraTransformComponent);
		mSkyRenderer.Update(cameraComponent, cameraTransformComponent, frameIdx);
		mImGuiRenderer.Update();
	}

	void VulkanRenderer::RecordTransfers(VulkanCommandRecorder& recorder, uInt32 frameIdx)
	{
		mSkyRenderer.RecordTransfers(recorder, frameIdx);
		mTerrainRenderer.RecordTransfers(recorder);
		mSceneRenderer.RecordTransfers(recorder, mBufferCache, frameIdx);
	}

	void VulkanRenderer::RecordPreDraws(VulkanCommandRecorder& recorder, uInt32 frameIdx)
	{
		mSkyRenderer.RecordPreDraws(recorder, frameIdx);
	}

	void VulkanRenderer::RecordDraws(VulkanCommandRecorder& recorder, uInt32 frameIdx)
	{
		mSkyRenderer.RecordDraws(recorder, frameIdx);
		mTerrainRenderer.RecordDraws(recorder);
		mSceneRenderer.RecordDraws(recorder, frameIdx);
		mImGuiRenderer.RecordDraws(recorder);
	}

	void VulkanRenderer::RecordPostDraws(VulkanCommandRecorder& recorder, uInt32 frameIdx)
	{

	}

	void VulkanRenderer::RenderScene(EntityWorld& world, uSize frameIdx)
	{
		VulkanCommandBuffer* pCommandBuffer = mCommandBuffers[frameIdx];
		VulkanCommandRecorder recorder(pCommandBuffer);

		VulkanSubmission renderSubmition	= {};
		renderSubmition.commandBuffers		= { mCommandBuffers[frameIdx] };
		renderSubmition.waitSemaphores		= { mSwapTimer.GetCurrentAcquiredSemaphore()}; //, mSkyRenderer.GetLUTsCompleteSemaphore(frameIdx) };
		renderSubmition.waitStages			= { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
		renderSubmition.signalSemaphores	= { mSwapTimer.GetCurrentCompleteSemaphore() };

		recorder.Reset();

		recorder.BeginRecording();

		RecordTransfers(recorder, frameIdx);

		RecordPreDraws(recorder, frameIdx);

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

		recorder.PipelineBarrierSwapchainImageBegin(mpSwapchain->images[frameIdx]);

		VkImageMemoryBarrier vkDepthImageMemoryBarrier = {};
		vkDepthImageMemoryBarrier.sType								= VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		vkDepthImageMemoryBarrier.srcAccessMask						= 0;
		vkDepthImageMemoryBarrier.dstAccessMask						= VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
		vkDepthImageMemoryBarrier.oldLayout							= VK_IMAGE_LAYOUT_UNDEFINED;
		vkDepthImageMemoryBarrier.newLayout							= VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		vkDepthImageMemoryBarrier.image								= mDepthImages[frameIdx]->vkImage;
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
		swapchainRenderingAttachmentInfo.pImageView		= mpSwapchain->imageViews[frameIdx];
		swapchainRenderingAttachmentInfo.imageLayout	= VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		swapchainRenderingAttachmentInfo.loadOp			= VK_ATTACHMENT_LOAD_OP_CLEAR;
		swapchainRenderingAttachmentInfo.storeOp		= VK_ATTACHMENT_STORE_OP_STORE;
		//swapchainRenderingAttachmentInfo.clearValue		= { 0.02f, 0.05f, 0.05f, 1.0f };
		swapchainRenderingAttachmentInfo.clearValue		= { 0.7f, 0.8f, 1.0f, 1.0f };

		VulkanRenderingAttachmentInfo depthRenderingAttachmentInfo = {};
		depthRenderingAttachmentInfo.pImageView		= mDepthImageViews[frameIdx];
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
		RecordDraws(recorder, frameIdx);
		recorder.EndRendering();

		RecordPostDraws(recorder, frameIdx);

		recorder.PipelineBarrierSwapchainImageEnd(mpSwapchain->images[frameIdx]);

		recorder.EndRecording();

		mpGraphics->Submit(renderSubmition, mpGraphics->pPrimaryDevice->queues.graphics, mSwapTimer.GetCurrentFence());

		mSwapTimer.Present();
	}

	void VulkanRenderer::RenderUpdate(Runtime& runtime, double delta)
	{
		EntityWorld& world = Engine::GetWorld();

		mSwapTimer.AdvanceFrame();
		uInt32 frameIdx = mSwapTimer.GetFrameIndex();

		UpdateAll(world, frameIdx);
		RenderScene(world, frameIdx);
	}

	void VulkanRenderer::Register(Runtime& runtime)
	{
		runtime.RegisterOnUpdate(&VulkanRenderer::RenderUpdate, this);
	}
}