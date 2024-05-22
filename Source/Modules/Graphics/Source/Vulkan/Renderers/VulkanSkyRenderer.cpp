#include "Vulkan/Renderers/VulkanSkyRenderer.h"

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
		mpSkyPerFrameData->_pad0_				= (float)'N';
		mpSkyPerFrameData->ozoneAbsorbtion		= atmosphere.ozoneAbsorbtion;
		mpSkyPerFrameData->_pad1_				= (float)'D';
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
		vkImageMemoryBarrier.dstAccessMask						= VK_ACCESS_SHADER_READ_BIT;
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
		barrierInfo.dstStage					= VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
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

#define LUT_FORMAT VK_FORMAT_R16G16B16A16_SFLOAT

	void VulkanSkyRenderer::Initialize(VulkanGraphics& graphics, VulkanDevice& device, const AtmosphereValues& atmosphere, const SkyRenderSettings& settings,
		VulkanShaderCache& shaderCache, VulkanPipelineCache& pipelineCache, uSize maxInFlightCount)
	{
		mpGraphics			= &graphics;
		mpResourceManager	= graphics.pResourceManager;
		mpDevice			= &device;
		mAtmosphere			= atmosphere;
		mSettings			= settings;

		/* Create Uniform Buffers */

		VulkanBufferInfo skyPerFrameStagingBufferInfo = {};
		skyPerFrameStagingBufferInfo.sizeBytes			= sizeof(AtmospherePerFrameData);
		skyPerFrameStagingBufferInfo.vkBufferUsage		= VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
		skyPerFrameStagingBufferInfo.vkMemoryProperties	= VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

		VulkanBufferInfo skyPerFrameBufferInfo = {};
		skyPerFrameBufferInfo.sizeBytes				= sizeof(AtmospherePerFrameData);
		skyPerFrameBufferInfo.vkBufferUsage			= VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
		skyPerFrameBufferInfo.vkMemoryProperties	= VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

		mpSkyPerFrameStagingBuffer = mpResourceManager->CreateBuffer(&device, skyPerFrameStagingBufferInfo);
		mpSkyPerFrameBuffer = mpResourceManager->CreateBuffer(&device, skyPerFrameBufferInfo);

		mpSkyPerFrameWriter = VulkanBufferWriter(mpSkyPerFrameStagingBuffer);
		mpSkyPerFrameData = mpSkyPerFrameWriter.Map<AtmospherePerFrameData>();

		/* Create LUT Images */

		for (uSize i = 0; i < maxInFlightCount; i++)
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

			mpSkyTransmittanceLUT[i] = mpResourceManager->CreateImage(&device, transmittanceImageInfo);
			mpSkyScatterLUT[i] = mpResourceManager->CreateImage(&device, scatterImageInfo);
			mpSkyViewLUT[i] = mpResourceManager->CreateImage(&device, viewImageInfo);

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

			mpSkyTransmittanceLUTView[i] = mpResourceManager->CreateImageView(&device, transmittanceViewInfo);
			mpSkyScatterLUTView[i] = mpResourceManager->CreateImageView(&device, scatterViewInfo);
			mpSkyViewLUTView[i] = mpResourceManager->CreateImageView(&device, viewViewInfo);
		}

		/* Create Samplers */

		VkSamplerCreateInfo lutSamplerInfo{};
		lutSamplerInfo.sType					= VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		lutSamplerInfo.magFilter				= VK_FILTER_LINEAR;
		lutSamplerInfo.minFilter				= VK_FILTER_LINEAR;
		lutSamplerInfo.addressModeU				= VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		lutSamplerInfo.addressModeV				= VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		lutSamplerInfo.addressModeW				= VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		lutSamplerInfo.anisotropyEnable			= VK_FALSE;
		lutSamplerInfo.maxAnisotropy			= 1;
		lutSamplerInfo.borderColor				= VK_BORDER_COLOR_INT_OPAQUE_BLACK;
		lutSamplerInfo.unnormalizedCoordinates	= VK_FALSE;
		lutSamplerInfo.compareEnable			= VK_FALSE;
		lutSamplerInfo.compareOp				= VK_COMPARE_OP_ALWAYS;
		lutSamplerInfo.mipmapMode				= VK_SAMPLER_MIPMAP_MODE_LINEAR;
		lutSamplerInfo.mipLodBias				= 0.0f;
		lutSamplerInfo.minLod					= 0.0f;
		lutSamplerInfo.maxLod					= 0.0f;

		// @TODO
		vkCreateSampler(device.vkDevice, &lutSamplerInfo, VK_NULL_HANDLE, &mVkLUTSampler);

		/* Create Semaphores */

		for (uSize i = 0; i < maxInFlightCount; i++)
		{
			VkSemaphoreTypeCreateInfo vkSemaphoreType = {};
			vkSemaphoreType.sType			= VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO;
			vkSemaphoreType.semaphoreType	= VK_SEMAPHORE_TYPE_BINARY;
			vkSemaphoreType.initialValue	= 0;

			VkSemaphoreCreateInfo semaphoreInfo = {};
			semaphoreInfo.sType				= VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
			semaphoreInfo.flags				= 0;
			semaphoreInfo.pNext				= &vkSemaphoreType;

			vkCreateSemaphore(device.vkDevice, &semaphoreInfo, nullptr, &mSkyTransmittanceLUTcomplete[i]);
			vkCreateSemaphore(device.vkDevice, &semaphoreInfo, nullptr, &mSkyScatterLUTcomplete[i]);
			vkCreateSemaphore(device.vkDevice, &semaphoreInfo, nullptr, &mSkyViewLUTcomplete[i]);
		}

		/* Create Shaders */

		VulkanShader* pFullscreenVertexShader				= shaderCache.FindOrCreateShader("Shaders/fullscreen.qsvert");
		VulkanShader* pSkyTransmittanceLUTFragmentShader	= shaderCache.FindOrCreateShader("Shaders/skyTransmittanceLUT.qsfrag");
		VulkanShader* pSkyScatterLUTFragmentShader			= shaderCache.FindOrCreateShader("Shaders/skyScatterLUT.qsfrag");
		VulkanShader* pSkyViewLUTFragmentShader				= shaderCache.FindOrCreateShader("Shaders/skyViewLUT.qsfrag");
		VulkanShader* pSkyRenderFragmentShader				= shaderCache.FindOrCreateShader("Shaders/sky.qsfrag");

		/* Create Shader Pipelines */

		Array<VulkanAttachment, 1> transmittanceAttachments =
		{
			{ "TransmittanceLUT", VULKAN_ATTACHMENT_TYPE_COLOR, LUT_FORMAT }
		};

		Array<VulkanAttachment, 1> scatterAttachments =
		{
			{ "ScatterLUT", VULKAN_ATTACHMENT_TYPE_COLOR, LUT_FORMAT }
		};

		Array<VulkanAttachment, 1> viewAttachments =
		{
			{ "ViewLUT", VULKAN_ATTACHMENT_TYPE_COLOR, LUT_FORMAT }
		};

		Array<VulkanAttachment, 2> attachments =
		{
			{ "Swapchain",			VULKAN_ATTACHMENT_TYPE_SWAPCHAIN,		VK_FORMAT_B8G8R8A8_UNORM },
			{ "Depth-Stencil",		VULKAN_ATTACHMENT_TYPE_DEPTH_STENCIL,	VK_FORMAT_D24_UNORM_S8_UINT }
		};

		VulkanGraphicsPipelineInfo skyTransmittanceLUTPipelineInfo = 
			pipelineCache.MakeGraphicsPipelineInfo(
			{ pFullscreenVertexShader, pSkyTransmittanceLUTFragmentShader }, transmittanceAttachments, {}, {});
		skyTransmittanceLUTPipelineInfo.depth.enableTesting				= false;
		skyTransmittanceLUTPipelineInfo.depth.enableWrite				= false;
		skyTransmittanceLUTPipelineInfo.stencil.enableTesting			= false;
		skyTransmittanceLUTPipelineInfo.blendAttachments[0].blendEnable = false;

		VulkanGraphicsPipelineInfo skyScatterLUTPipelineInfo =
			pipelineCache.MakeGraphicsPipelineInfo(
				{ pFullscreenVertexShader, pSkyScatterLUTFragmentShader }, scatterAttachments, {}, {});
		skyScatterLUTPipelineInfo.depth.enableTesting					= false;
		skyScatterLUTPipelineInfo.depth.enableWrite						= false;
		skyScatterLUTPipelineInfo.stencil.enableTesting					= false;
		skyScatterLUTPipelineInfo.blendAttachments[0].blendEnable		= false;

		VulkanGraphicsPipelineInfo skyViewLUTPipelineInfo =
			pipelineCache.MakeGraphicsPipelineInfo(
				{ pFullscreenVertexShader, pSkyViewLUTFragmentShader }, viewAttachments, {}, {});
		skyViewLUTPipelineInfo.depth.enableTesting						= false;
		skyViewLUTPipelineInfo.depth.enableWrite						= false;
		skyViewLUTPipelineInfo.stencil.enableTesting					= false;
		skyViewLUTPipelineInfo.blendAttachments[0].blendEnable			= false;

		VulkanGraphicsPipelineInfo skyRenderPipelineInfo =
			pipelineCache.MakeGraphicsPipelineInfo(
				{ pFullscreenVertexShader, pSkyRenderFragmentShader }, attachments, {}, {});
		skyRenderPipelineInfo.depth.enableTesting						= false;
		skyRenderPipelineInfo.stencil.enableTesting						= false;

		mpSkyTransmittanceLUTPipeline = pipelineCache.FindOrCreateGraphicsPipeline(skyTransmittanceLUTPipelineInfo);
		mpSkyScatterLUTPipeline = pipelineCache.FindOrCreateGraphicsPipeline(skyScatterLUTPipelineInfo);
		mpSkyViewLUTPipeline = pipelineCache.FindOrCreateGraphicsPipeline(skyViewLUTPipelineInfo);
		mpSkyRenderPipeline = pipelineCache.FindOrCreateGraphicsPipeline(skyRenderPipelineInfo);
	}

	Quatf sunRotation;

	void VulkanSkyRenderer::Update(CameraComponent& camera, TransformComponent& cameraTransform, uSize frameIdx)
	{
		mAtmosphere.suns[0].sunDir = sunRotation * mAtmosphere.suns[0].sunDir;
		sunRotation = Quatf().SetAxisAngle(Vec3f(1.0f, 0.0f, 0.0f), ToRadians(-1.0f * 0.001f));
		sunRotation.Normalize();

		CopyAtmospherePerFrameData(mAtmosphere, cameraTransform.position, cameraTransform.GetForward(), camera.width, camera.height);
	}

	void VulkanSkyRenderer::RecordTransfers(VulkanCommandRecorder& transferRecorder, uSize frameIdx)
	{
		transferRecorder.CopyBuffer(mpSkyPerFrameStagingBuffer, mpSkyPerFrameBuffer, mpSkyPerFrameBuffer->sizeBytes, 0, 0);
	}

	void VulkanSkyRenderer::RenderLUTs(VulkanCommandRecorder& recorder, uSize frameIdx)
	{
		// @TODO: Turn off blending in pipeline (and culling)

		/* Setup Structs */

		VkViewport vkViewport = {};
		vkViewport.x		= 0;
		vkViewport.y		= 0;
		vkViewport.width	= 0;
		vkViewport.height	= 0;
		vkViewport.minDepth = 0.0f;
		vkViewport.maxDepth = 1.0f;

		VkRect2D vkScissor = {};
		vkScissor.offset.x		= 0;
		vkScissor.offset.y		= 0;
		vkScissor.extent.width	= 0;
		vkScissor.extent.height = 0;

		VulkanUniformBufferBind perFrameBinding = {};
		perFrameBinding.binding		= 0;
		perFrameBinding.pBuffer		= mpSkyPerFrameBuffer;
		perFrameBinding.offset		= 0;
		perFrameBinding.range		= mpSkyPerFrameBuffer->sizeBytes;

		VulkanUniformBufferBind pBufferBinds[] = { perFrameBinding };

		VulkanRenderingAttachmentInfo transmittanceLUTAttachmentInfo = {};
		transmittanceLUTAttachmentInfo.pImageView		= mpSkyTransmittanceLUTView[frameIdx];
		transmittanceLUTAttachmentInfo.imageLayout		= VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		transmittanceLUTAttachmentInfo.loadOp			= VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		transmittanceLUTAttachmentInfo.storeOp			= VK_ATTACHMENT_STORE_OP_STORE;
		transmittanceLUTAttachmentInfo.clearValue		= { 0.0f, 0.0f, 0.0f };

		VulkanRenderingAttachmentInfo scatterLUTAttachmentInfo = {};
		scatterLUTAttachmentInfo.pImageView				= mpSkyScatterLUTView[frameIdx];
		scatterLUTAttachmentInfo.imageLayout			= VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		scatterLUTAttachmentInfo.loadOp					= VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		scatterLUTAttachmentInfo.storeOp				= VK_ATTACHMENT_STORE_OP_STORE;
		scatterLUTAttachmentInfo.clearValue				= { 0.0f, 0.0f, 0.0f };

		VulkanRenderingAttachmentInfo viewLUTAttachmentInfo = {};
		viewLUTAttachmentInfo.pImageView				= mpSkyViewLUTView[frameIdx];
		viewLUTAttachmentInfo.imageLayout				= VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		viewLUTAttachmentInfo.loadOp					= VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		viewLUTAttachmentInfo.storeOp					= VK_ATTACHMENT_STORE_OP_STORE;
		viewLUTAttachmentInfo.clearValue				= { 0.0f, 0.0f, 0.0f };

		VulkanUniformImageBind transmittanceLUTBinding = {};
		transmittanceLUTBinding.binding		= 1;
		transmittanceLUTBinding.vkSampler	= mVkLUTSampler;
		transmittanceLUTBinding.pImageView	= mpSkyTransmittanceLUTView[frameIdx];
		transmittanceLUTBinding.vkLayout	= VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

		VulkanUniformImageBind scatterLUTBinding = {};
		scatterLUTBinding.binding			= 1;
		scatterLUTBinding.vkSampler			= mVkLUTSampler;
		scatterLUTBinding.pImageView		= mpSkyScatterLUTView[frameIdx];
		scatterLUTBinding.vkLayout			= VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

		VulkanUniformImageBind viewLUTBinding = {};
		viewLUTBinding.binding				= 1;
		viewLUTBinding.vkSampler			= mVkLUTSampler;
		viewLUTBinding.pImageView			= mpSkyViewLUTView[frameIdx];
		viewLUTBinding.vkLayout				= VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

		VulkanRenderingBeginInfo lutBeginInfo = {};
		lutBeginInfo.pColorAttachments		= nullptr;
		lutBeginInfo.colorAttachmentCount	= 1;
		lutBeginInfo.pDepthAttachment		= nullptr;
		lutBeginInfo.pStencilAttachment		= nullptr;
		lutBeginInfo.renderArea				= {};

		/* Render Transmittance LUT */

		VulkanRenderingAttachmentInfo pTransmittanceAttachmentInfos[] = { transmittanceLUTAttachmentInfo };

		lutBeginInfo.pColorAttachments	= pTransmittanceAttachmentInfos;
		lutBeginInfo.renderArea			= { { 0, 0 }, { mSettings.transmittanceLUTSize.x, mSettings.transmittanceLUTSize.y } };

		vkViewport.x					= 0;
		vkViewport.y					= mSettings.transmittanceLUTSize.y;
		vkViewport.width				= mSettings.transmittanceLUTSize.x;
		vkViewport.height				= -(float)mSettings.transmittanceLUTSize.y;

		vkScissor.extent.width			= mSettings.transmittanceLUTSize.x;
		vkScissor.extent.height			= mSettings.transmittanceLUTSize.y;

		recorder.BeginRendering(lutBeginInfo);
		recorder.SetViewport(vkViewport, vkScissor);
		recorder.SetGraphicsPipeline(mpSkyTransmittanceLUTPipeline);
		recorder.BindUniforms(mpSkyTransmittanceLUTPipeline, 0, pBufferBinds, 1, nullptr, 0);
		recorder.Draw(1, 3, 0);
		recorder.EndRendering();

		/* Render Scatter LUT */

		VulkanRenderingAttachmentInfo pScatterAttachmentInfos[] = { scatterLUTAttachmentInfo };
		lutBeginInfo.pColorAttachments	= pScatterAttachmentInfos;
		lutBeginInfo.renderArea			= { { 0, 0 }, { mSettings.scatterLUTSize.x, mSettings.scatterLUTSize.y } };
		
		vkViewport.x					= 0;
		vkViewport.y					= mSettings.scatterLUTSize.y;
		vkViewport.width				= mSettings.scatterLUTSize.x;
		vkViewport.height				= -(float)mSettings.scatterLUTSize.y;
		
		vkScissor.extent.width			= mSettings.scatterLUTSize.x;
		vkScissor.extent.height			= mSettings.scatterLUTSize.y;
		 
		recorder.BeginRendering(lutBeginInfo);
		recorder.SetViewport(vkViewport, vkScissor);
		recorder.SetGraphicsPipeline(mpSkyScatterLUTPipeline);
		
		VulkanUniformImageBind pScatterLUTImageBinds[] = { transmittanceLUTBinding };
		
		recorder.BindUniforms(mpSkyScatterLUTPipeline, 0, pBufferBinds, 1, pScatterLUTImageBinds, 1);
		recorder.Draw(1, 3, 0);
		recorder.EndRendering();

		/* Render View LUT */

		VulkanRenderingAttachmentInfo pViewAttachmentInfos[] = { viewLUTAttachmentInfo };
		lutBeginInfo.pColorAttachments	= pViewAttachmentInfos;
		lutBeginInfo.renderArea			= { { 0, 0 }, { mSettings.viewLUTSize.x, mSettings.viewLUTSize.y } };

		vkViewport.x					= 0;
		vkViewport.y					= mSettings.viewLUTSize.y;
		vkViewport.width				= mSettings.viewLUTSize.x;
		vkViewport.height				= -(float)mSettings.viewLUTSize.y;

		vkScissor.extent.width			= mSettings.viewLUTSize.x;
		vkScissor.extent.height			= mSettings.viewLUTSize.y;

		recorder.BeginRendering(lutBeginInfo);
		recorder.SetViewport(vkViewport, vkScissor);
		recorder.SetGraphicsPipeline(mpSkyViewLUTPipeline);

		scatterLUTBinding.binding = 2;
		VulkanUniformImageBind pViewLUTImageBinds[] = { transmittanceLUTBinding, scatterLUTBinding };

		recorder.BindUniforms(mpSkyViewLUTPipeline, 0, pBufferBinds, 1, pViewLUTImageBinds, 2);
		recorder.Draw(1, 3, 0);
		recorder.EndRendering();

		PrepareImageForSampling(recorder, mpSkyTransmittanceLUT[frameIdx]);
		PrepareImageForSampling(recorder, mpSkyScatterLUT[frameIdx]);
		PrepareImageForSampling(recorder, mpSkyViewLUT[frameIdx]);
	}

	void VulkanSkyRenderer::RecordPreDraws(VulkanCommandRecorder& renderRecorder, uSize frameIdx)
	{
		RenderLUTs(renderRecorder, frameIdx);
	}

	void VulkanSkyRenderer::RecordDraws(VulkanCommandRecorder& renderRecorder, uSize frameIdx)
	{
		renderRecorder.SetGraphicsPipeline(mpSkyRenderPipeline);

		VulkanUniformBufferBind binding = {};
		binding.binding = 0;
		binding.pBuffer = mpSkyPerFrameBuffer;
		binding.offset	= 0;
		binding.range	= mpSkyPerFrameBuffer->sizeBytes;

		VulkanUniformImageBind viewLUTBinding = {};
		viewLUTBinding.binding				= 1;
		viewLUTBinding.vkSampler			= mVkLUTSampler;
		viewLUTBinding.pImageView			= mpSkyViewLUTView[frameIdx];
		viewLUTBinding.vkLayout				= VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

		VulkanUniformImageBind transmittanceLUTBinding = {};
		transmittanceLUTBinding.binding		= 2;
		transmittanceLUTBinding.vkSampler	= mVkLUTSampler;
		transmittanceLUTBinding.pImageView	= mpSkyTransmittanceLUTView[frameIdx];
		transmittanceLUTBinding.vkLayout	= VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

		VulkanUniformBufferBind pBufferBinds[] = { binding };
		VulkanUniformImageBind pImageBinds[] = { viewLUTBinding, transmittanceLUTBinding };
			
		renderRecorder.BindUniforms(mpSkyRenderPipeline, 0, pBufferBinds, 1, pImageBinds, 1);

		renderRecorder.Draw(1, 3, 0);
	}

	VkSemaphore VulkanSkyRenderer::GetLUTsCompleteSemaphore(uSize frameIdx)
	{
		return mSkyTransmittanceLUTcomplete[frameIdx];
	}
}

