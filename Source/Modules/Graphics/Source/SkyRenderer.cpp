#include "SkyRenderer.h"

namespace Quartz
{
	void VulkanSkyRenderer::CopyAtmospherePerFrameData(const AtmosphereValues& atmosphere, 
		const Vec3f& cameraPos, const Vec3f viewDir, float width, float height)
	{
		mpSkyPerFrameData->rayleighScattering	= atmosphere.rayleighScattering;
		mpSkyPerFrameData->rayleighAbsorbtion	= atmosphere.rayleighAbsorbtion;
		mpSkyPerFrameData->mieScattering		= atmosphere.mieScattering;
		mpSkyPerFrameData->mieAbsorbtion		= atmosphere.mieAbsorbtion;
		mpSkyPerFrameData->ozoneScattering		= atmosphere.ozoneScattering;
		mpSkyPerFrameData->_pad0_				= (float)'H';
		mpSkyPerFrameData->ozoneAbsorbtion		= atmosphere.ozoneAbsorbtion;
		mpSkyPerFrameData->_pad1_				= (float)'I';
		mpSkyPerFrameData->suns[0]				= atmosphere.suns[0];
		mpSkyPerFrameData->suns[1]				= atmosphere.suns[1];

		mpSkyPerFrameData->cameraPos	= cameraPos;
		mpSkyPerFrameData->viewDir		= viewDir;
		mpSkyPerFrameData->width		= width;
		mpSkyPerFrameData->height		= height;
	}

	void VulkanSkyRenderer::PrepareImageForRender(VulkanCommandRecorder& recorder, VulkanImage* pImage)
	{
		VkImageMemoryBarrier vkImageMemoryBarrier = {};
		vkImageMemoryBarrier.sType								= VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		vkImageMemoryBarrier.srcAccessMask						= 0;
		vkImageMemoryBarrier.dstAccessMask						= VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		vkImageMemoryBarrier.oldLayout							= VK_IMAGE_LAYOUT_UNDEFINED;
		vkImageMemoryBarrier.newLayout							= VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		vkImageMemoryBarrier.image								= pImage->vkImage;
		vkImageMemoryBarrier.subresourceRange.aspectMask		= VK_IMAGE_ASPECT_COLOR_BIT;
		vkImageMemoryBarrier.subresourceRange.baseMipLevel		= 0;
		vkImageMemoryBarrier.subresourceRange.levelCount		= 1;
		vkImageMemoryBarrier.subresourceRange.baseArrayLayer	= 0;
		vkImageMemoryBarrier.subresourceRange.layerCount		= 1;

		VulkanPipelineBarrierInfo barrierInfo = {};
		barrierInfo.srcStage					= VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		barrierInfo.dstStage					= VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		barrierInfo.dependencyFlags				= 0;
		barrierInfo.memoryBarrierCount			= 0;
		barrierInfo.pMemoryBarriers				= nullptr;
		barrierInfo.bufferMemoryBarrierCount	= 0;
		barrierInfo.pBufferMemoryBarriers		= nullptr;
		barrierInfo.imageMemoryBarrierCount		= 1;
		barrierInfo.pImageMemoryBarriers		= &vkImageMemoryBarrier;

		vkCmdPipelineBarrier(
			recorder.GetCommandBuffer().vkCommandBuffer,
			barrierInfo.srcStage,
			barrierInfo.dstStage,
			barrierInfo.dependencyFlags,
			barrierInfo.memoryBarrierCount,
			barrierInfo.pMemoryBarriers,
			barrierInfo.bufferMemoryBarrierCount,
			barrierInfo.pBufferMemoryBarriers,
			barrierInfo.imageMemoryBarrierCount,
			barrierInfo.pImageMemoryBarriers);
	}

	void VulkanSkyRenderer::PrepareImageForSampling(VulkanCommandRecorder& recorder, VulkanImage* pImage)
	{
		VkImageMemoryBarrier vkImageMemoryBarrier = {};
		vkImageMemoryBarrier.sType								= VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		vkImageMemoryBarrier.srcAccessMask						= 0;
		vkImageMemoryBarrier.dstAccessMask						= VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		vkImageMemoryBarrier.oldLayout							= VK_IMAGE_LAYOUT_UNDEFINED;
		vkImageMemoryBarrier.newLayout							= VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		vkImageMemoryBarrier.image								= pImage->vkImage;
		vkImageMemoryBarrier.subresourceRange.aspectMask		= VK_IMAGE_ASPECT_COLOR_BIT;
		vkImageMemoryBarrier.subresourceRange.baseMipLevel		= 0;
		vkImageMemoryBarrier.subresourceRange.levelCount		= 1;
		vkImageMemoryBarrier.subresourceRange.baseArrayLayer	= 0;
		vkImageMemoryBarrier.subresourceRange.layerCount		= 1;

		VulkanPipelineBarrierInfo barrierInfo = {};
		barrierInfo.srcStage					= VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		barrierInfo.dstStage					= VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
		barrierInfo.dependencyFlags				= 0;
		barrierInfo.memoryBarrierCount			= 0;
		barrierInfo.pMemoryBarriers				= nullptr;
		barrierInfo.bufferMemoryBarrierCount	= 0;
		barrierInfo.pBufferMemoryBarriers		= nullptr;
		barrierInfo.imageMemoryBarrierCount		= 1;
		barrierInfo.pImageMemoryBarriers		= &vkImageMemoryBarrier;

		vkCmdPipelineBarrier(
			recorder.GetCommandBuffer().vkCommandBuffer,
			barrierInfo.srcStage,
			barrierInfo.dstStage,
			barrierInfo.dependencyFlags,
			barrierInfo.memoryBarrierCount,
			barrierInfo.pMemoryBarriers,
			barrierInfo.bufferMemoryBarrierCount,
			barrierInfo.pBufferMemoryBarriers,
			barrierInfo.imageMemoryBarrierCount,
			barrierInfo.pImageMemoryBarriers);
	}

#define LUT_FORMAT VK_FORMAT_R8G8B8A8_UNORM

	void VulkanSkyRenderer::Initialize(VulkanGraphics& graphics, const AtmosphereValues& atmosphere, const SkyRenderSettings& settings, 
		VulkanShaderCache& shaderCache, VulkanPipelineCache& pipelineCache)
	{
		VulkanResourceManager& resources = *graphics.pResourceManager;
		VulkanDevice& device = *graphics.pPrimaryDevice;

		mpGraphics = &graphics;
		mAtmosphere = atmosphere;
		mSettings = settings;

		/* Create Command Buffers */

		VulkanCommandPoolInfo skyCommandPoolInfo = {};
		skyCommandPoolInfo.queueFamilyIndex			= graphics.pPrimaryDevice->pPhysicalDevice->primaryQueueFamilyIndices.graphics;
		skyCommandPoolInfo.vkCommandPoolCreateFlags	= VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

		mpImmediateCommandPool = graphics.pResourceManager->CreateCommandPool(graphics.pPrimaryDevice, skyCommandPoolInfo);
		graphics.pResourceManager->CreateCommandBuffers(mpImmediateCommandPool, 3, mImmediateCommandBuffers);

		VkFenceCreateInfo vkImmediateFenceInfo = {};
		vkImmediateFenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		vkImmediateFenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
		vkImmediateFenceInfo.pNext = nullptr;

		for (uSize i = 0; i < 3; i++)
		{
			mImmediateRecorders[i] = VulkanCommandRecorder(mImmediateCommandBuffers[i]);
			vkCreateFence(graphics.pPrimaryDevice->vkDevice, &vkImmediateFenceInfo, VK_NULL_HANDLE, &mImmediateFences[i]);
		}

		/* Create Uniform Buffers */

		VulkanBufferInfo skyPerFrameStagingBufferInfo = {};
		skyPerFrameStagingBufferInfo.sizeBytes			= sizeof(AtmospherePerFrameData);
		skyPerFrameStagingBufferInfo.vkBufferUsage		= VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
		skyPerFrameStagingBufferInfo.vkMemoryProperties	= VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

		VulkanBufferInfo skyPerFrameBufferInfo = {};
		skyPerFrameBufferInfo.sizeBytes				= sizeof(AtmospherePerFrameData);
		skyPerFrameBufferInfo.vkBufferUsage			= VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
		skyPerFrameBufferInfo.vkMemoryProperties	= VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

		mpSkyPerFrameStagingBuffer = resources.CreateBuffer(&device, skyPerFrameStagingBufferInfo);
		mpSkyPerFrameBuffer = resources.CreateBuffer(&device, skyPerFrameBufferInfo);

		mpSkyPerFrameWriter = VulkanBufferWriter(mpSkyPerFrameStagingBuffer);
		mpSkyPerFrameData = mpSkyPerFrameWriter.Map<AtmospherePerFrameData>();

		/* Create LUT Images */

		for (uSize i = 0; i < 3; i++)
		{
			VulkanImageInfo transmittanceImageInfo = {};
			transmittanceImageInfo.vkImageType	= VK_IMAGE_TYPE_2D;
			transmittanceImageInfo.vkFormat		= LUT_FORMAT;
			transmittanceImageInfo.vkUsageFlags	= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
			transmittanceImageInfo.width		= settings.transmittanceLUTSize.x;
			transmittanceImageInfo.height		= settings.transmittanceLUTSize.y;
			transmittanceImageInfo.depth		= 1;
			transmittanceImageInfo.layers		= 1;
			transmittanceImageInfo.mips			= 1;

			VulkanImageInfo scatterImageInfo = {};
			scatterImageInfo.vkImageType		= VK_IMAGE_TYPE_2D;
			scatterImageInfo.vkFormat			= LUT_FORMAT;
			scatterImageInfo.vkUsageFlags		= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
			scatterImageInfo.width				= settings.scatterLUTSize.x;
			scatterImageInfo.height				= settings.scatterLUTSize.y;
			scatterImageInfo.depth				= 1;
			scatterImageInfo.layers				= 1;
			scatterImageInfo.mips				= 1;

			VulkanImageInfo viewImageInfo = {};
			viewImageInfo.vkImageType			= VK_IMAGE_TYPE_2D;
			viewImageInfo.vkFormat				= LUT_FORMAT;
			viewImageInfo.vkUsageFlags			= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
			viewImageInfo.width					= settings.viewLUTSize.x;
			viewImageInfo.height				= settings.viewLUTSize.y;
			viewImageInfo.depth					= 1;
			viewImageInfo.layers				= 1;
			viewImageInfo.mips					= 1;

			mpSkyTransmittanceLUT[i] = resources.CreateImage(&device, transmittanceImageInfo);
			mpSkyScatterLUT[i] = resources.CreateImage(&device, scatterImageInfo);
			mpSkyViewLUT[i] = resources.CreateImage(&device, viewImageInfo);

			VulkanImageViewInfo transmittanceViewInfo = {};
			transmittanceViewInfo.pImage				= mpSkyTransmittanceLUT[i];
			transmittanceViewInfo.vkImageViewType		= VK_IMAGE_VIEW_TYPE_2D;
			transmittanceViewInfo.vkAspectFlags			= VK_IMAGE_ASPECT_COLOR_BIT;
			transmittanceViewInfo.vkFormat				= LUT_FORMAT;
			transmittanceViewInfo.mipStart				= 0;
			transmittanceViewInfo.mipCount				= 1;
			transmittanceViewInfo.layerStart			= 0;
			transmittanceViewInfo.layerCount			= 1;

			VulkanImageViewInfo scatterViewInfo = {};
			scatterViewInfo.pImage						= mpSkyScatterLUT[i];
			scatterViewInfo.vkImageViewType				= VK_IMAGE_VIEW_TYPE_2D;
			scatterViewInfo.vkAspectFlags				= VK_IMAGE_ASPECT_COLOR_BIT;
			scatterViewInfo.vkFormat					= LUT_FORMAT;
			scatterViewInfo.mipStart					= 0;
			scatterViewInfo.mipCount					= 1;
			scatterViewInfo.layerStart					= 0;
			scatterViewInfo.layerCount					= 1;

			VulkanImageViewInfo viewViewInfo = {};
			viewViewInfo.pImage							= mpSkyViewLUT[i];
			viewViewInfo.vkImageViewType				= VK_IMAGE_VIEW_TYPE_2D;
			viewViewInfo.vkAspectFlags					= VK_IMAGE_ASPECT_COLOR_BIT;
			viewViewInfo.vkFormat						= LUT_FORMAT;
			viewViewInfo.mipStart						= 0;
			viewViewInfo.mipCount						= 1;
			viewViewInfo.layerStart						= 0;
			viewViewInfo.layerCount						= 1;

			mpSkyTransmittanceLUTView[i] = resources.CreateImageView(&device, transmittanceViewInfo);
			mpSkyScatterLUTView[i] = resources.CreateImageView(&device, scatterViewInfo);
			mpSkyViewLUTView[i] = resources.CreateImageView(&device, viewViewInfo);
		}

		/* Create Semaphores */

		for (uSize i = 0; i < 3; i++)
		{
			VkSemaphoreTypeCreateInfo vkSemaphoreType = {};
			vkSemaphoreType.sType			= VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO;
			vkSemaphoreType.semaphoreType	= VK_SEMAPHORE_TYPE_BINARY;
			vkSemaphoreType.initialValue	= 0;

			VkSemaphoreCreateInfo semaphoreInfo = {};
			semaphoreInfo.sType				= VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
			semaphoreInfo.flags				= 0;
			semaphoreInfo.pNext				= &vkSemaphoreType;

			VkFenceCreateInfo fenceInfo = {};
			fenceInfo.sType					= VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
			fenceInfo.flags					= VK_FENCE_CREATE_SIGNALED_BIT; // Start signaled
			fenceInfo.pNext					= nullptr;

			vkCreateSemaphore(device.vkDevice, &semaphoreInfo, nullptr, &mSkyTransmittanceLUTcomplete[i]);
			vkCreateSemaphore(device.vkDevice, &semaphoreInfo, nullptr, &mSkyScatterLUTcomplete[i]);
			vkCreateSemaphore(device.vkDevice, &semaphoreInfo, nullptr, &mSkyViewLUTcomplete[i]);
		}

		/* Create Shader Pipelines */

		VulkanShader* pFullscreenVertexShader				= shaderCache.FindOrCreateShader("Shaders/fullscreen.vert", VK_SHADER_STAGE_VERTEX_BIT);
		VulkanShader* pSkyTransmittanceLUTFragmentShader	= shaderCache.FindOrCreateShader("Shaders/skyTransmittanceLUT.frag", VK_SHADER_STAGE_FRAGMENT_BIT);
		VulkanShader* pSkyScatterLUTFragmentShader			= shaderCache.FindOrCreateShader("Shaders/skyScatterLUT.frag", VK_SHADER_STAGE_FRAGMENT_BIT);
		VulkanShader* pSkyViewLUTFragmentShader				= shaderCache.FindOrCreateShader("Shaders/skyViewLUT.frag", VK_SHADER_STAGE_FRAGMENT_BIT);
		VulkanShader* pSkyRenderFragmentShader				= shaderCache.FindOrCreateShader("Shaders/sky.frag", VK_SHADER_STAGE_FRAGMENT_BIT);

		Array<VulkanAttachment> transmittanceAttachments =
		{
			{ "TransmittanceLUT", VULKAN_ATTACHMENT_TYPE_COLOR, LUT_FORMAT }
		};

		mpSkyTransmittanceLUTPipeline = pipelineCache.FindOrCreateGraphicsPipeline(
			{ pFullscreenVertexShader, pSkyTransmittanceLUTFragmentShader }, transmittanceAttachments, {}, {});

		Array<VulkanAttachment> scatterAttachments =
		{
			{ "ScatterLUT", VULKAN_ATTACHMENT_TYPE_COLOR, LUT_FORMAT }
		};

		mpSkyScatterLUTPipeline = pipelineCache.FindOrCreateGraphicsPipeline(
			{ pFullscreenVertexShader, pSkyScatterLUTFragmentShader }, scatterAttachments, {}, {});

		Array<VulkanAttachment> viewAttachments =
		{
			{ "ViewLUT", VULKAN_ATTACHMENT_TYPE_COLOR, LUT_FORMAT }
		};

		mpSkyViewLUTPipeline = pipelineCache.FindOrCreateGraphicsPipeline(
			{ pFullscreenVertexShader, pSkyViewLUTFragmentShader }, viewAttachments, {}, {});

		Array<VulkanAttachment> attachments =
		{
			{ "Swapchain",			VULKAN_ATTACHMENT_TYPE_SWAPCHAIN,		VK_FORMAT_B8G8R8A8_UNORM },
			{ "Depth-Stencil",		VULKAN_ATTACHMENT_TYPE_DEPTH_STENCIL,	VK_FORMAT_D24_UNORM_S8_UINT }
		};

		// All geometry will be generated in the vertex shader
		mpSkyRenderPipeline = pipelineCache.FindOrCreateGraphicsPipeline(
			{ pFullscreenVertexShader, pSkyRenderFragmentShader }, attachments, {}, {});
	}

	void VulkanSkyRenderer::Update(CameraComponent& camera, TransformComponent& cameraTransform, uSize frameIdx)
	{
		CopyAtmospherePerFrameData(mAtmosphere, cameraTransform.position, cameraTransform.GetForward(), camera.width, camera.height);
		RenderLUTs(frameIdx);
	}

	void VulkanSkyRenderer::RecordTransfers(VulkanCommandRecorder& transferRecorder, uSize frameIdx)
	{
		transferRecorder.CopyBuffer(mpSkyPerFrameStagingBuffer, mpSkyPerFrameBuffer, mpSkyPerFrameBuffer->sizeBytes, 0, 0);
	}

	void VulkanSkyRenderer::RenderLUTs(uSize frameIdx)
	{
		VulkanDevice& device = *mpGraphics->pPrimaryDevice;

		VulkanCommandRecorder& immediateRecorder = mImmediateRecorders[frameIdx];
		VulkanCommandBuffer* pImmediateCommandBuffer = mImmediateCommandBuffers[frameIdx];
		VkFence& vkImmediateFence = mImmediateFences[frameIdx];

		vkWaitForFences(device.vkDevice, 1, &vkImmediateFence, true, UINT64_MAX);
		vkResetFences(device.vkDevice, 1, &vkImmediateFence);

		immediateRecorder.Reset();
		immediateRecorder.BeginRecording();

		VulkanRenderingAttachmentInfo transmittanceLUTAttachmentInfo = {};
		transmittanceLUTAttachmentInfo.pImageView		= mpSkyTransmittanceLUTView[frameIdx];
		transmittanceLUTAttachmentInfo.imageLayout		= VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		transmittanceLUTAttachmentInfo.loadOp			= VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		transmittanceLUTAttachmentInfo.storeOp			= VK_ATTACHMENT_STORE_OP_STORE;
		transmittanceLUTAttachmentInfo.clearValue		= { 0.0f, 0.0f, 0.0f };

		VulkanRenderingAttachmentInfo pColorAttachmentInfos[] = { transmittanceLUTAttachmentInfo };

		VulkanRenderingBeginInfo renderingBeginInfo = {};
		renderingBeginInfo.pColorAttachments	= pColorAttachmentInfos;
		renderingBeginInfo.colorAttachmentCount	= 1;
		renderingBeginInfo.pDepthAttachment		= nullptr;
		renderingBeginInfo.pStencilAttachment	= nullptr;
		renderingBeginInfo.renderArea			= { { 0, 0 }, { mSettings.transmittanceLUTSize.x, mSettings.transmittanceLUTSize.y } };

		immediateRecorder.BeginRendering(renderingBeginInfo);

		VkViewport vkViewport = {};
		vkViewport.x		= 0;
		vkViewport.y		= mSettings.transmittanceLUTSize.y;
		vkViewport.width	= mSettings.transmittanceLUTSize.x;
		vkViewport.height	= -(float)mSettings.transmittanceLUTSize.y;
		vkViewport.minDepth = 0.0f;
		vkViewport.maxDepth = 1.0f;

		VkRect2D vkScissor = {};
		vkScissor.offset.x		= 0;
		vkScissor.offset.y		= 0;
		vkScissor.extent.width	= mSettings.transmittanceLUTSize.x;
		vkScissor.extent.height = mSettings.transmittanceLUTSize.y;

		immediateRecorder.SetViewport(vkViewport, vkScissor);

		immediateRecorder.SetGraphicsPipeline(mpSkyTransmittanceLUTPipeline);

		VulkanUniformBufferBind binding = {};
		binding.binding		= 0;
		binding.pBuffer		= mpSkyPerFrameBuffer;
		binding.offset		= 0;
		binding.range		= mpSkyPerFrameBuffer->sizeBytes;

		VulkanUniformBufferBind pBufferBinds[] = { binding };

		immediateRecorder.BindUniforms(mpSkyRenderPipeline, 0, pBufferBinds, 1, nullptr, 0);

		immediateRecorder.Draw(1, 3, 0);

		immediateRecorder.EndRendering();

		immediateRecorder.EndRecording();

		/* Submit Command Buffers */

		VulkanSubmission renderSubmition = {};
		renderSubmition.commandBuffers = { pImmediateCommandBuffer };
		renderSubmition.waitSemaphores = {};
		renderSubmition.waitStages = {};
		renderSubmition.signalSemaphores = { mSkyTransmittanceLUTcomplete[frameIdx] };

		mpGraphics->Submit(renderSubmition, device.queues.graphics, vkImmediateFence);
	}

	void VulkanSkyRenderer::RecordDraws(VulkanCommandRecorder& renderRecorder, uSize frameIdx)
	{
		renderRecorder.SetGraphicsPipeline(mpSkyRenderPipeline);

		VulkanUniformBufferBind binding = {};
		binding.binding = 0;
		binding.pBuffer = mpSkyPerFrameBuffer;
		binding.offset	= 0;
		binding.range	= mpSkyPerFrameBuffer->sizeBytes;

		VulkanUniformBufferBind pBufferBinds[] = { binding };
			
		renderRecorder.BindUniforms(mpSkyRenderPipeline, 0, pBufferBinds, 1, nullptr, 0);

		renderRecorder.Draw(1, 3, 0);
	}
}

