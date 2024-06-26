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

	VulkanRenderer::VulkanRenderer(VulkanGraphics& graphics, VulkanDevice& device, Window& activeWindow, uSize maxInFlightCount)
	{
		assert(maxInFlightCount < VULKAN_GRAPHICS_MAX_IN_FLIGHT);

		mpGraphics			= &graphics;
		mpResourceManager	= graphics.pResourceManager;
		mpWindow			= &activeWindow;
		mpDevice			= &device;
		mMaxInFlightCount	= maxInFlightCount;

		mPipelineCache		= VulkanPipelineCache(mpDevice, mpResourceManager);
		mShaderCache		= VulkanShaderCache(mpDevice, mpResourceManager);
		mpSwapchain			= mpResourceManager->CreateSwapchain(mpDevice, *graphics.pSurface, maxInFlightCount);
		mSwapTimer			= VulkanSwapchainTimer(mpSwapchain);
	}

	void VulkanRenderer::Initialize()
	{
		// Color Pass

		VulkanRenderSettings renderSettings = {};
		renderSettings.useUniqueMeshBuffers				= false;
		renderSettings.useUniqueMeshStagingBuffers		= false;
		renderSettings.useUniqueUniformBuffers			= false;
		renderSettings.useUniqueUniformStagingBuffers	= false;
		renderSettings.vertexBufferSizeMb				= 512; // 32
		renderSettings.indexBufferSizeMb				= 512; // 16
		renderSettings.perInstanceBufferSizeMb			= 64;  // 16
		renderSettings.uniformBufferSizeMb				= 64;
		renderSettings.globalBufferSizeBytes			= 128;
		renderSettings.uniquePerInstanceBufferSizeBytes	= 128; //
		renderSettings.uniquePerModelBufferSizeBytes	= 128; //
		renderSettings.maxUniformSets					= 4;
		renderSettings.useInstancing					= false;
		renderSettings.useMeshStaging					= true;
		renderSettings.useUniformStaging				= true;
		renderSettings.useDrawIndirect					= false;

		mBufferCache.Initialize(mpDevice, mpResourceManager, renderSettings);

		for (uSize i = 0; i < mMaxInFlightCount; i++)
		{
			VulkanImageInfo colorImageInfo = {};
			colorImageInfo.vkFormat		= VK_FORMAT_R32G32B32A32_SFLOAT;
			colorImageInfo.vkImageType	= VK_IMAGE_TYPE_2D;
			colorImageInfo.vkUsageFlags = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
			colorImageInfo.width		= mpGraphics->pSurface->width;
			colorImageInfo.height		= mpGraphics->pSurface->height;
			colorImageInfo.depth		= 1;
			colorImageInfo.layers		= 1;
			colorImageInfo.mips			= 1;

			mColorImages[i] = mpResourceManager->CreateImage(mpDevice, colorImageInfo);

			VulkanImageViewInfo colorImageViewInfo = {};
			colorImageViewInfo.pImage			= mColorImages[i];
			colorImageViewInfo.vkFormat			= VK_FORMAT_R32G32B32A32_SFLOAT;
			colorImageViewInfo.vkImageViewType	= VK_IMAGE_VIEW_TYPE_2D;
			colorImageViewInfo.vkAspectFlags	= VK_IMAGE_ASPECT_COLOR_BIT;
			colorImageViewInfo.layerStart		= 0;
			colorImageViewInfo.layerCount		= 1;
			colorImageViewInfo.mipStart			= 0;
			colorImageViewInfo.mipCount			= 1;

			mColorImageViews[i] = mpResourceManager->CreateImageView(mpDevice, colorImageViewInfo);

			VulkanImageInfo depthImageInfo = {};
			depthImageInfo.vkFormat		= VK_FORMAT_D24_UNORM_S8_UINT;
			depthImageInfo.vkImageType	= VK_IMAGE_TYPE_2D;
			depthImageInfo.vkUsageFlags = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
			depthImageInfo.width		= mpGraphics->pSurface->width;
			depthImageInfo.height		= mpGraphics->pSurface->height;
			depthImageInfo.depth		= 1;
			depthImageInfo.layers		= 1;
			depthImageInfo.mips			= 1;

			mDepthImages[i] = mpResourceManager->CreateImage(mpDevice, depthImageInfo);

			VulkanImageViewInfo depthImageViewInfo = {};
			depthImageViewInfo.pImage			= mDepthImages[i];
			depthImageViewInfo.vkFormat			= VK_FORMAT_D24_UNORM_S8_UINT;
			depthImageViewInfo.vkImageViewType	= VK_IMAGE_VIEW_TYPE_2D;
			depthImageViewInfo.vkAspectFlags	= VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
			depthImageViewInfo.layerStart		= 0;
			depthImageViewInfo.layerCount		= 1;
			depthImageViewInfo.mipStart			= 0;
			depthImageViewInfo.mipCount			= 1;

			mDepthImageViews[i] = mpResourceManager->CreateImageView(mpDevice, depthImageViewInfo);
		}

		VulkanCommandPoolInfo renderPoolInfo = {};
		renderPoolInfo.queueFamilyIndex			= mpDevice->pPhysicalDevice-> primaryQueueFamilyIndices.graphics;
		renderPoolInfo.vkCommandPoolCreateFlags	= VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

		VulkanCommandPool* pRenderPool = mpResourceManager->CreateCommandPool(mpDevice, renderPoolInfo);

		mpResourceManager->CreateCommandBuffers(pRenderPool, mMaxInFlightCount, mCommandBuffers);

		mTerrainRenderer.Initialize(*mpGraphics, *mpDevice, mShaderCache, mPipelineCache, mMaxInFlightCount);

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
		settings.transmittanceLUTSize	= { 256, 64 };
		settings.scatterLUTSize			= { 32, 32 };
		settings.viewLUTSize			= { 200, 200 };

		mSceneRenderer.Initialize(*mpGraphics, *mpDevice, mShaderCache, mPipelineCache, mMaxInFlightCount);
		mSkyRenderer.Initialize(*mpGraphics, *mpDevice, atmosphere, settings, mShaderCache, mPipelineCache, mMaxInFlightCount);

		VkPipelineRenderingCreateInfo renderingInfo = {};
		renderingInfo.sType						= VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;
		renderingInfo.colorAttachmentCount		= 1;
		renderingInfo.pColorAttachmentFormats	= &mColorImages[0]->vkFormat;
		renderingInfo.depthAttachmentFormat		= VK_FORMAT_D24_UNORM_S8_UINT;

		mImGuiRenderer.Initialize(*mpGraphics, *mpDevice, *mpWindow, renderingInfo);

		// Tonemap Pass

		VulkanShader* pTonemapVertexShader		= mShaderCache.FindOrCreateShader("Shaders/fullscreen.qsvert");
		VulkanShader* pTonemapFragmentShader	= mShaderCache.FindOrCreateShader("Shaders/tonemap_hdr-sdr.qsfrag");

		Array<VulkanAttachment, 1> tonemapPassAttachments =
		{
			{ "Swapchain", VULKAN_ATTACHMENT_TYPE_SWAPCHAIN, VK_FORMAT_B8G8R8A8_UNORM }
		};

		VulkanGraphicsPipelineInfo tonemapPipelineInfo =
			mPipelineCache.MakeGraphicsPipelineInfo(
				{ pTonemapVertexShader, pTonemapFragmentShader }, tonemapPassAttachments);

		tonemapPipelineInfo.vkCullMode = VK_CULL_MODE_NONE;

		mpTonemapPipeline = mPipelineCache.FindOrCreateGraphicsPipeline(tonemapPipelineInfo);

		VkSamplerCreateInfo tonemapSamplerInfo{};
		tonemapSamplerInfo.sType					= VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		tonemapSamplerInfo.magFilter				= VK_FILTER_LINEAR;
		tonemapSamplerInfo.minFilter				= VK_FILTER_LINEAR;
		tonemapSamplerInfo.addressModeU				= VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		tonemapSamplerInfo.addressModeV				= VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		tonemapSamplerInfo.addressModeW				= VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		tonemapSamplerInfo.anisotropyEnable			= VK_FALSE;
		tonemapSamplerInfo.maxAnisotropy			= 1;
		tonemapSamplerInfo.borderColor				= VK_BORDER_COLOR_INT_OPAQUE_BLACK;
		tonemapSamplerInfo.unnormalizedCoordinates	= VK_FALSE;
		tonemapSamplerInfo.compareEnable			= VK_FALSE;
		tonemapSamplerInfo.compareOp				= VK_COMPARE_OP_ALWAYS;
		tonemapSamplerInfo.mipmapMode				= VK_SAMPLER_MIPMAP_MODE_LINEAR;
		tonemapSamplerInfo.mipLodBias				= 0.0f;
		tonemapSamplerInfo.minLod					= 0.0f;
		tonemapSamplerInfo.maxLod					= 0.0f;

		vkCreateSampler(mpDevice->vkDevice, &tonemapSamplerInfo, VK_NULL_HANDLE, &mVkTonemapSampler);
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

	void VulkanRenderer::UpdateAll(EntityWorld& world, uSize frameIdx, double deltaTime)
	{
		TransformComponent& cameraTransformComponent = world.Get<TransformComponent>(mCameraEntity);
		CameraComponent& cameraComponent = world.Get<CameraComponent>(mCameraEntity);

		mSceneRenderer.Update(world, mBufferCache, mShaderCache, mPipelineCache, cameraComponent, cameraTransformComponent, frameIdx);

		Vec2f centerPos = { cameraTransformComponent.position.x, cameraTransformComponent.position.z };
		mTerrainRenderer.Update(centerPos, cameraComponent, cameraTransformComponent);
		mSkyRenderer.Update(cameraComponent, cameraTransformComponent, frameIdx);
		mImGuiRenderer.Update(this, deltaTime);
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

	void VulkanRenderer::RecordTonemapDraws(VulkanCommandRecorder& recorder, uInt32 frameIdx)
	{
		recorder.SetGraphicsPipeline(mpTonemapPipeline);

		VulkanUniformImageBind colorImageBind = {};
		colorImageBind.binding		= 0;
		colorImageBind.pImageView	= mColorImageViews[frameIdx];
		colorImageBind.vkLayout		= VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		colorImageBind.vkSampler	= mVkTonemapSampler;

		recorder.BindUniforms(mpTonemapPipeline, 0, nullptr, 0, &colorImageBind, 1);

		recorder.Draw(1, 3, 0);
	}

	void VulkanRenderer::RenderScene(EntityWorld& world, uSize frameIdx)
	{
		VulkanCommandBuffer* pCommandBuffer = mCommandBuffers[frameIdx];
		VulkanCommandRecorder recorder(pCommandBuffer);

		VulkanSubmission renderSubmition	= {};
		renderSubmition.commandBuffers		= { mCommandBuffers[frameIdx] };
		renderSubmition.waitSemaphores		= { mSwapTimer.GetCurrentAcquiredSemaphore()};
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

		// Color Pass

		VkImageMemoryBarrier vkColorImageMemoryBarrier = {};
		vkColorImageMemoryBarrier.sType								= VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		vkColorImageMemoryBarrier.srcAccessMask						= 0;
		vkColorImageMemoryBarrier.dstAccessMask						= VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		vkColorImageMemoryBarrier.oldLayout							= VK_IMAGE_LAYOUT_UNDEFINED;
		vkColorImageMemoryBarrier.newLayout							= VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		vkColorImageMemoryBarrier.image								= mColorImages[frameIdx]->vkImage;
		vkColorImageMemoryBarrier.subresourceRange.aspectMask		= VK_IMAGE_ASPECT_COLOR_BIT;
		vkColorImageMemoryBarrier.subresourceRange.baseMipLevel		= 0;
		vkColorImageMemoryBarrier.subresourceRange.levelCount		= 1;
		vkColorImageMemoryBarrier.subresourceRange.baseArrayLayer	= 0;
		vkColorImageMemoryBarrier.subresourceRange.layerCount		= 1;

		VulkanPipelineBarrierInfo colorImageBarrierInfo = {};
		colorImageBarrierInfo.srcStage					= VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		colorImageBarrierInfo.dstStage					= VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		colorImageBarrierInfo.dependencyFlags			= 0;
		colorImageBarrierInfo.memoryBarrierCount		= 0;
		colorImageBarrierInfo.pMemoryBarriers			= nullptr;
		colorImageBarrierInfo.bufferMemoryBarrierCount	= 0;
		colorImageBarrierInfo.pBufferMemoryBarriers		= nullptr;
		colorImageBarrierInfo.imageMemoryBarrierCount	= 1;
		colorImageBarrierInfo.pImageMemoryBarriers		= &vkColorImageMemoryBarrier;

		recorder.PipelineBarrier(colorImageBarrierInfo);

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

		VulkanPipelineBarrierInfo depthImageBarrierInfo = {};
		depthImageBarrierInfo.srcStage					= VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
		depthImageBarrierInfo.dstStage					= VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
		depthImageBarrierInfo.dependencyFlags			= 0;
		depthImageBarrierInfo.memoryBarrierCount		= 0;
		depthImageBarrierInfo.pMemoryBarriers			= nullptr;
		depthImageBarrierInfo.bufferMemoryBarrierCount	= 0;
		depthImageBarrierInfo.pBufferMemoryBarriers		= nullptr;
		depthImageBarrierInfo.imageMemoryBarrierCount	= 1;
		depthImageBarrierInfo.pImageMemoryBarriers		= &vkDepthImageMemoryBarrier;

		recorder.PipelineBarrier(depthImageBarrierInfo);

		VulkanRenderingAttachmentInfo colorRenderingAttachmentInfo = {};
		colorRenderingAttachmentInfo.pImageView		= mColorImageViews[frameIdx];
		colorRenderingAttachmentInfo.imageLayout	= VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		colorRenderingAttachmentInfo.loadOp			= VK_ATTACHMENT_LOAD_OP_CLEAR;
		colorRenderingAttachmentInfo.storeOp		= VK_ATTACHMENT_STORE_OP_STORE;
		colorRenderingAttachmentInfo.clearValue		= { 0.02f, 0.05f, 0.05f, 1.0f };

		VulkanRenderingAttachmentInfo depthRenderingAttachmentInfo = {};
		depthRenderingAttachmentInfo.pImageView		= mDepthImageViews[frameIdx];
		depthRenderingAttachmentInfo.imageLayout	= VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		depthRenderingAttachmentInfo.loadOp			= VK_ATTACHMENT_LOAD_OP_CLEAR;
		depthRenderingAttachmentInfo.storeOp		= VK_ATTACHMENT_STORE_OP_STORE;
		depthRenderingAttachmentInfo.clearValue		= { 1.0f, 0 };

		VulkanRenderingAttachmentInfo pColorAttachmentInfos[] = { colorRenderingAttachmentInfo };

		VulkanRenderingBeginInfo renderingBeginInfo = {};
		renderingBeginInfo.pColorAttachments	= pColorAttachmentInfos;
		renderingBeginInfo.colorAttachmentCount	= 1;
		renderingBeginInfo.pDepthAttachment		= &depthRenderingAttachmentInfo;
		renderingBeginInfo.pStencilAttachment	= nullptr;
		renderingBeginInfo.renderArea			= { { 0, 0 }, { mpGraphics->pSurface->width, mpGraphics->pSurface->height } };

		recorder.BeginRendering(renderingBeginInfo);
		RecordDraws(recorder, frameIdx);
		recorder.EndRendering();

		VkImageMemoryBarrier vkColorImageEndMemoryBarrier = {};
		vkColorImageEndMemoryBarrier.sType								= VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		vkColorImageEndMemoryBarrier.srcAccessMask						= VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		vkColorImageEndMemoryBarrier.dstAccessMask						= 0;
		vkColorImageEndMemoryBarrier.oldLayout							= VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		vkColorImageEndMemoryBarrier.newLayout							= VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		vkColorImageEndMemoryBarrier.image								= mColorImages[frameIdx]->vkImage;
		vkColorImageEndMemoryBarrier.subresourceRange.aspectMask		= VK_IMAGE_ASPECT_COLOR_BIT;
		vkColorImageEndMemoryBarrier.subresourceRange.baseMipLevel		= 0;
		vkColorImageEndMemoryBarrier.subresourceRange.levelCount		= 1;
		vkColorImageEndMemoryBarrier.subresourceRange.baseArrayLayer	= 0;
		vkColorImageEndMemoryBarrier.subresourceRange.layerCount		= 1;

		VulkanPipelineBarrierInfo colorImageEndBarrierInfo = {};
		colorImageEndBarrierInfo.srcStage					= VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		colorImageEndBarrierInfo.dstStage					= VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		colorImageEndBarrierInfo.dependencyFlags			= 0;
		colorImageEndBarrierInfo.memoryBarrierCount			= 0;
		colorImageEndBarrierInfo.pMemoryBarriers			= nullptr;
		colorImageEndBarrierInfo.bufferMemoryBarrierCount	= 0;
		colorImageEndBarrierInfo.pBufferMemoryBarriers		= nullptr;
		colorImageEndBarrierInfo.imageMemoryBarrierCount	= 1;
		colorImageEndBarrierInfo.pImageMemoryBarriers		= &vkColorImageEndMemoryBarrier;

		recorder.PipelineBarrier(colorImageEndBarrierInfo);

		// Tonemap Pass

		recorder.PipelineBarrierSwapchainImageBegin(mpSwapchain->images[frameIdx]);

		VulkanRenderingAttachmentInfo swapchainRenderingAttachmentInfo = {};
		swapchainRenderingAttachmentInfo.pImageView		= mpSwapchain->imageViews[frameIdx];
		swapchainRenderingAttachmentInfo.imageLayout	= VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		swapchainRenderingAttachmentInfo.loadOp			= VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		swapchainRenderingAttachmentInfo.storeOp		= VK_ATTACHMENT_STORE_OP_STORE;

		VulkanRenderingAttachmentInfo pSwapchainAttachmentInfos[] = { swapchainRenderingAttachmentInfo };

		VulkanRenderingBeginInfo tonemapRenderingBeginInfo = {};
		tonemapRenderingBeginInfo.pColorAttachments		= pSwapchainAttachmentInfos;
		tonemapRenderingBeginInfo.colorAttachmentCount	= 1;
		tonemapRenderingBeginInfo.pDepthAttachment		= nullptr;
		tonemapRenderingBeginInfo.pStencilAttachment	= nullptr;
		tonemapRenderingBeginInfo.renderArea			= { { 0, 0 }, { mpGraphics->pSurface->width, mpGraphics->pSurface->height } };

		recorder.BeginRendering(tonemapRenderingBeginInfo);
		RecordTonemapDraws(recorder, frameIdx);
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

		UpdateAll(world, mCurrentFrameIdx, delta);

		mAccumFrametime += delta;
		if (mAccumFrametime >= (1.0 / mTargetFPS))
		{
			mSwapTimer.AdvanceFrame();
			mCurrentFrameIdx = mSwapTimer.GetFrameIndex();

			mCurrentFPS = (1.0 / mAccumFrametime);
			mAverageFPS = mAverageDecayFPS * mAverageFPS + (1.0 - mAverageDecayFPS) * mCurrentFPS;
			mAccumFrametime = 0;

			RenderScene(world, mCurrentFrameIdx);
		}
	}

	void VulkanRenderer::Register(Runtime& runtime)
	{
		runtime.RegisterOnUpdate(&VulkanRenderer::RenderUpdate, this);
	}

	void VulkanRenderer::SetTargetFPS(uInt64 fps)
	{
		mTargetFPS = fps;
	}
}