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

		// VERTEX DATA

		float vertexData[] =
		{
			-0.5f, -0.5f, 0.0f,  1.0f, 0.0f, 0.0f,
			 0.0f,  0.5f, 0.0f,  0.0f, 1.0f, 0.0f,
			 0.5f, -0.5f, 0.0f,  0.0f, 0.0f, 1.0f
		};

		VulkanBufferInfo stagingVertexBufferInfo		= {};
		stagingVertexBufferInfo.sizeBytes				= sizeof(vertexData);
		stagingVertexBufferInfo.vkBufferUsage			= VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
		stagingVertexBufferInfo.vkMemoryProperties		= VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

		VulkanBuffer* pStagingVertexBuffer = pResources->CreateBuffer(pGraphics->pPrimaryDevice, stagingVertexBufferInfo);

		VulkanBufferInfo vertexBufferInfo	= {};
		vertexBufferInfo.sizeBytes			= sizeof(vertexData);
		vertexBufferInfo.vkBufferUsage		= VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
		vertexBufferInfo.vkMemoryProperties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

		VulkanBuffer* pVertexBuffer = pResources->CreateBuffer(pGraphics->pPrimaryDevice, vertexBufferInfo);

		VulkanBufferWriter vertexWriter(pStagingVertexBuffer);
		float* pVertexData = vertexWriter.Map<float>();

		memcpy_s(pVertexData, vertexBufferInfo.sizeBytes, vertexData, vertexBufferInfo.sizeBytes);

		vertexWriter.Unmap();


		// INDEX DATA

		uInt16 indexData[] =
		{
			0, 1, 2
		};

		VulkanBufferInfo stagingIndexBufferInfo		= {};
		stagingIndexBufferInfo.sizeBytes			= sizeof(indexData);
		stagingIndexBufferInfo.vkBufferUsage		= VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
		stagingIndexBufferInfo.vkMemoryProperties	= VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

		VulkanBuffer* pStagingIndexBuffer = pResources->CreateBuffer(pGraphics->pPrimaryDevice, stagingIndexBufferInfo);

		VulkanBufferInfo indexBufferInfo	= {};
		indexBufferInfo.sizeBytes			= sizeof(indexData);
		indexBufferInfo.vkBufferUsage		= VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
		indexBufferInfo.vkMemoryProperties	= VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

		VulkanBuffer* pIndexBuffer = pResources->CreateBuffer(pGraphics->pPrimaryDevice, indexBufferInfo);

		VulkanBufferWriter indexWriter(pStagingIndexBuffer);
		uInt16* pIndexData = indexWriter.Map<uInt16>();

		memcpy_s(pIndexData, indexBufferInfo.sizeBytes, indexData, indexBufferInfo.sizeBytes);

		indexWriter.Unmap();


		// Uniform Buffers

		uSize transformUniformBufferSize = pVertexShader->uniforms[0].sizeBytes;

		for (uSize i = 0; i < 3; i++)
		{
			VulkanBufferInfo stagingUniformBufferInfo	= {};
			stagingUniformBufferInfo.sizeBytes			= transformUniformBufferSize;
			stagingUniformBufferInfo.vkBufferUsage		= VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
			stagingUniformBufferInfo.vkMemoryProperties	= VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;

			VulkanBuffer* pStagingUniformBuffer = pResources->CreateBuffer(pGraphics->pPrimaryDevice, stagingUniformBufferInfo);

			VulkanBufferInfo uniformBufferInfo		= {};
			uniformBufferInfo.sizeBytes				= transformUniformBufferSize;
			uniformBufferInfo.vkBufferUsage			= VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
			uniformBufferInfo.vkMemoryProperties	= VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

			VulkanBuffer* pUniformBuffer = pResources->CreateBuffer(pGraphics->pPrimaryDevice, uniformBufferInfo);

			VulkanBufferWriter uniformWriter(pStagingUniformBuffer);
			TransformUniformData* pTransformData = uniformWriter.Map<TransformUniformData>();

			pTransformData->transform = Mat4f().SetRotation({0.0f, 1.0f, 0.0f}, 0.5f);
			pTransformData->view = Mat4f().SetTranslation({ 0.0f, 0.0f, -1.0f }) * Mat4f().SetPerspective(ToRadians(90.0f),
				(float)pGraphics->pSurface->width / (float)pGraphics->pSurface->height, 0.001f, 1000.0f);

			mUniformTransformStagingBuffers[i] = pStagingUniformBuffer;
			mUniformTransformBuffers[i] = pUniformBuffer;
			mUniformTransformWriters[i] = uniformWriter;
			mTransformData[i] = pTransformData;

			//uniformWriter.Unmap();
		}

		// Command Buffers

		VulkanCommandPoolInfo immediateTransferPoolInfo = {};
		immediateTransferPoolInfo.queueFamilyIndex			= pGraphics->pPrimaryDevice->pPhysicalDevice->primaryQueueFamilyIndices.transfer;
		immediateTransferPoolInfo.vkCommandPoolCreateFlags	= VK_COMMAND_POOL_CREATE_TRANSIENT_BIT | VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

		VulkanCommandPool* pImmedateTransferPool = pResources->CreateCommandPool(pGraphics->pPrimaryDevice, immediateTransferPoolInfo);

		VulkanCommandBuffer* pImmediateCommandBuffer;
		pResources->CreateCommandBuffers(pImmedateTransferPool, 1, &pImmediateCommandBuffer);

		VulkanCommandRecorder immediateRecorder(pImmediateCommandBuffer);

		immediateRecorder.BeginRecording();

		immediateRecorder.CopyBuffer(pStagingVertexBuffer, pVertexBuffer, pVertexBuffer->sizeBytes, 0, 0);
		immediateRecorder.CopyBuffer(pStagingIndexBuffer, pIndexBuffer, pIndexBuffer->sizeBytes, 0, 0);

		for (uSize i = 0; i < 3; i++)
		{
			immediateRecorder.CopyBuffer(mUniformTransformStagingBuffers[i], mUniformTransformBuffers[i], 128, 0, 0);
		}

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

		pResources->DestroyBuffer(pStagingVertexBuffer);
		pResources->DestroyBuffer(pStagingIndexBuffer);

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

			VulkanBufferBind pVertexBufferBinds[] = { {pVertexBuffer, 0} };

			renderRecorder.SetVertexBuffers(pVertexBufferBinds, 1);
			renderRecorder.SetIndexBuffer(pIndexBuffer, 0, VK_INDEX_TYPE_UINT16);
			renderRecorder.SetGraphicsPipeline(mpPipeline);

			VulkanUniformBind binding = {};
			binding.binding = 0;
			binding.pBuffer = mUniformTransformBuffers[i];
			binding.offset	= 0;
			binding.range	= mUniformTransformBuffers[i]->sizeBytes;

			VulkanUniformBind pUniformBinds[] = { binding };

			renderRecorder.BindUniforms(mpPipeline, 0, pUniformBinds, 1);

			renderRecorder.DrawIndexed(1, pIndexBuffer->sizeBytes / sizeof(uInt16), 0);

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