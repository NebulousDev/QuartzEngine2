#include "Vulkan/VulkanRenderer.h"

#include "Log.h"
#include "Vulkan/VulkanGraphics.h"
#include "Vulkan/Primatives/VulkanPipeline.h"
#include "Vulkan/VulkanCommandRecorder.h"
#include "Vulkan/VulkanSwapchainTimer.h"
#include "Vulkan/VulkanBufferWriter.h"
#include "Vulkan/VulkanRenderScene.h"

#include "Vulkan/VulkanMultiBuffer.h"

#include "shaderc/shaderc.hpp"

#include "Component/MeshComponent.h"
#include "Component/TransformComponent.h"

namespace Quartz
{
	void CompileShader(const String& shaderName, const char* source, Array<uInt8>& spirv, shaderc_shader_kind kind)
	{
		shaderc::Compiler compiler;
		shaderc::CompileOptions options;

		LogInfo("Compiling Shader '%s'...", shaderName.Str());

		options.SetOptimizationLevel(shaderc_optimization_level_performance);

		shaderc::SpvCompilationResult module =
			compiler.CompileGlslToSpv(source, kind, "shaderc-shader", options);

		if (module.GetCompilationStatus() != shaderc_compilation_status_success)
		{
			LogError("Failed compiling shader '%s':\n%s", shaderName.Str(), module.GetErrorMessage().c_str());
		}

		std::vector<uInt32> binary = { module.cbegin(), module.cend() };
		spirv.Resize(binary.size() * sizeof(unsigned int));
		memcpy_s(spirv.Data(), spirv.Size(), binary.data(), binary.size() * sizeof(unsigned int));
	}

	void VulkanRenderer::Initialize(VulkanGraphics* pGraphics)
	{
		VulkanResourceManager*	pResources	= pGraphics->pResourceManager;
		VulkanDevice*			pDevice		= pGraphics->pPrimaryDevice;

		mpGraphics = pGraphics;


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

		VulkanRenderScene renderScene;
		renderScene.Initialize(pDevice, pResources, renderSettings);

		renderScene.BuildScene(mpGraphics->pEntityWorld);


		// SWAPCHAIN

		mpSwapchain = pResources->CreateSwapchain(pGraphics->pPrimaryDevice, *pGraphics->pSurface, 3);		
		mSwapTimer = VulkanSwapchainTimer(mpSwapchain);


		// SHADERS

		char* vertexShader = 
		R"(
			#version 450
			#extension GL_ARB_separate_shader_objects : enable
			
			layout(location = 0) in vec3 inPosition;
			layout(location = 1) in vec3 inNormal;
			//layout(location = 2) in vec3 inTangent;
			//layout(location = 3) in vec2 inTexCoord;
			
			layout(location = 0) out vec3 outNormal;

			layout(set = 0, binding = 0) uniform TransformUBO
			{
				mat4 model;
				mat4 view;
			} ubo;

			void main()
			{
				outNormal = inNormal;
				gl_Position = (ubo.view * ubo.model) * vec4(inPosition, 1.0);
			}
		)";

		char* fragmentShader =
		R"(
			#version 450
			#extension GL_ARB_separate_shader_objects : enable
			#extension GL_KHR_vulkan_glsl : enable
			
			layout(location = 0) in vec3 inNormal;

			layout(location = 0) out vec4 fragOut;
			
			void main()
			{
				fragOut = vec4(inNormal, 1.0);
			}
		)";

		Array<uInt8> vertexShaderSPIRV;
		Array<uInt8> fragmentShaderSPIRV;

		CompileShader("Vertex", vertexShader, vertexShaderSPIRV, shaderc_shader_kind::shaderc_glsl_vertex_shader);
		CompileShader("Fragment", fragmentShader, fragmentShaderSPIRV, shaderc_shader_kind::shaderc_glsl_fragment_shader);

		VulkanShader* pVertexShader = pResources->CreateShader(pGraphics->pPrimaryDevice, "Vertex", vertexShaderSPIRV);
		VulkanShader* pFragmentShader = pResources->CreateShader(pGraphics->pPrimaryDevice, "Fragment", fragmentShaderSPIRV);

		mPipelineManager = VulkanPipelineManager(pDevice, pResources);

		// DEPTH IMAGES

		for (uSize i = 0; i < 3; i++)
		{
			VulkanImageInfo depthImageInfo = {};
			depthImageInfo.vkFormat		= VK_FORMAT_D24_UNORM_S8_UINT;
			depthImageInfo.vkImageType	= VK_IMAGE_TYPE_2D;
			depthImageInfo.vkUsageFlags	= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
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


		// PIPELINE

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

		mpPipeline = mPipelineManager.FindOrCreateGraphicsPipeline(
			{ pVertexShader, pFragmentShader },
			attachments, vertexAttributes, vertexBindings
		);

		// Command Buffers

		VulkanCommandPoolInfo immediateTransferPoolInfo = {};
		immediateTransferPoolInfo.queueFamilyIndex			= pGraphics->pPrimaryDevice->pPhysicalDevice->primaryQueueFamilyIndices.transfer;
		immediateTransferPoolInfo.vkCommandPoolCreateFlags	= VK_COMMAND_POOL_CREATE_TRANSIENT_BIT | VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

		VulkanCommandPool* pImmedateTransferPool = pResources->CreateCommandPool(pGraphics->pPrimaryDevice, immediateTransferPoolInfo);

		VulkanCommandBuffer* pImmediateCommandBuffer;
		pResources->CreateCommandBuffers(pImmedateTransferPool, 1, &pImmediateCommandBuffer);

		VulkanCommandRecorder immediateRecorder(pImmediateCommandBuffer);

		immediateRecorder.BeginRecording();

		////
		renderScene.RecordTransfers(&immediateRecorder);
		////

		immediateRecorder.EndRecording();

		VulkanSubmission immediateSubmition	= {};
		immediateSubmition.commandBuffers	= { pImmediateCommandBuffer };
		immediateSubmition.waitSemaphores	= {};
		immediateSubmition.waitStages		= {};
		immediateSubmition.signalSemaphores	= {};

		pGraphics->Submit(immediateSubmition, pGraphics->pPrimaryDevice->queues.transfer, VK_NULL_HANDLE);

		VulkanCommandPoolInfo renderPoolInfo = {};
		renderPoolInfo.queueFamilyIndex			= pGraphics->pPrimaryDevice->pPhysicalDevice-> primaryQueueFamilyIndices.graphics;
		renderPoolInfo.vkCommandPoolCreateFlags	= VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

		VulkanCommandPool* pRenderPool = pResources->CreateCommandPool(pGraphics->pPrimaryDevice, renderPoolInfo);

		pResources->CreateCommandBuffers(pRenderPool, 3, mCommandBuffers);

		for (uSize i = 0; i < 3; i++)
		{
			VulkanCommandRecorder renderRecorder(mCommandBuffers[i]);

			renderRecorder.BeginRecording();

			renderRecorder.PipelineBarrierSwapchainImageBegin(mpSwapchain->images[i]);


			// DEPTH TRANSITION
			VkImageMemoryBarrier vkDepthImageMemoryBarrier = {};
			vkDepthImageMemoryBarrier.sType								= VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
			vkDepthImageMemoryBarrier.srcAccessMask						= 0;
			vkDepthImageMemoryBarrier.dstAccessMask						= VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
			vkDepthImageMemoryBarrier.oldLayout							= VK_IMAGE_LAYOUT_UNDEFINED;
			vkDepthImageMemoryBarrier.newLayout							= VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
			vkDepthImageMemoryBarrier.image								= mDepthImages[i]->vkImage;
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

			renderRecorder.PipelineBarrier(barrierInfo);

			VkViewport vkViewport = {};
			vkViewport.x		= 0;
			vkViewport.y		= pGraphics->pSurface->height;
			vkViewport.width	= pGraphics->pSurface->width;
			vkViewport.height	= -(float)pGraphics->pSurface->height;
			vkViewport.minDepth = 0.0f;
			vkViewport.maxDepth = 1.0f;

			VkRect2D vkScissor = {};
			vkScissor.offset.x		= 0;
			vkScissor.offset.y		= 0;
			vkScissor.extent.width	= pGraphics->pSurface->width;
			vkScissor.extent.height = pGraphics->pSurface->height;

			renderRecorder.SetViewport(vkViewport, vkScissor);

			VulkanRenderingAttachmentInfo swapchainRenderingAttachmentInfo = {};
			swapchainRenderingAttachmentInfo.pImageView		= mpSwapchain->imageViews[i];
			swapchainRenderingAttachmentInfo.imageLayout	= VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
			swapchainRenderingAttachmentInfo.loadOp			= VK_ATTACHMENT_LOAD_OP_CLEAR;
			swapchainRenderingAttachmentInfo.storeOp		= VK_ATTACHMENT_STORE_OP_STORE;
			swapchainRenderingAttachmentInfo.clearValue		= { 0.02f, 0.05f, 0.05f, 1.0f };

			VulkanRenderingAttachmentInfo depthRenderingAttachmentInfo = {};
			depthRenderingAttachmentInfo.pImageView		= mDepthImageViews[i];
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
			renderingBeginInfo.renderArea			= { { 0, 0 }, {pGraphics->pSurface->width, pGraphics->pSurface->height} };

			renderRecorder.BeginRendering(renderingBeginInfo);

			renderRecorder.SetGraphicsPipeline(mpPipeline);

			renderScene.RecordRender(&renderRecorder, mpPipeline);

			renderRecorder.EndRendering();

			renderRecorder.PipelineBarrierSwapchainImageEnd(mpSwapchain->images[i]);

			renderRecorder.EndRecording();
		}
	}

	void VulkanRenderer::RenderScene(VulkanRenderScene* pRenderScene)
	{
		mSwapTimer.AdvanceFrame();

		uInt32 resourceIdx = mSwapTimer.GetFrameIndex();

		VulkanSubmission renderSubmition	= {};
		renderSubmition.commandBuffers		= { mCommandBuffers[resourceIdx] };
		renderSubmition.waitSemaphores		= { mSwapTimer.GetCurrentAcquiredSemaphore() };
		renderSubmition.waitStages			= { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
		renderSubmition.signalSemaphores	= { mSwapTimer.GetCurrentCompleteSemaphore() };

		mpGraphics->Submit(renderSubmition, mpGraphics->pPrimaryDevice->queues.graphics, mSwapTimer.GetCurrentFence());

		mSwapTimer.Present();
	}

	void VulkanRenderer::RenderUpdate(Runtime* pRuntime, double delta)
	{
		RenderScene(nullptr);
	}

	void VulkanRenderer::Register(Runtime* pRuntime)
	{
		pRuntime->RegisterOnUpdate(&VulkanRenderer::RenderUpdate, this);
	}
}