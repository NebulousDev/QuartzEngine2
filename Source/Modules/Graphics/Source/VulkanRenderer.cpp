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


		{
			VulkanBufferInfo testBufferInfo	= {};
			testBufferInfo.sizeBytes				= 256;
			testBufferInfo.vkBufferUsage			= VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
			testBufferInfo.vkMemoryProperties		= VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

			VulkanBuffer* pTestBuffer = pResources->CreateBuffer(pGraphics->pPrimaryDevice, testBufferInfo);

			VulkanMultiBuffer muliBuffer(pTestBuffer);
			muliBuffer.Map();

			VulkanMultiBufferEntry entry1;
			VulkanMultiBufferEntry entry2;
			VulkanMultiBufferEntry entry3;
			VulkanMultiBufferEntry entry4;
			VulkanMultiBufferEntry entry5;

			muliBuffer.Allocate<float>(120 / 4, entry1, nullptr);
			muliBuffer.Allocate<float>(120 / 4, entry2, nullptr);
			muliBuffer.Allocate<float>(16 / 4,  entry3, nullptr);

			muliBuffer.Free(entry2);

			muliBuffer.Allocate<float>(64 / 4, entry4, nullptr);
			muliBuffer.Allocate<float>(16 / 4, entry5, nullptr);

			muliBuffer.Free(entry4);
			muliBuffer.Free(entry5);

			muliBuffer.Unmap();
		}




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
		renderSettings.useInstancing					= true;
		renderSettings.useMeshStaging					= true;
		renderSettings.useUniformStaging				= true;
		renderSettings.useDrawIndirect					= false;

		VulkanRenderScene renderScene;
		renderScene.Initialize(pDevice, pResources, renderSettings);

		renderScene.BuildScene(mpGraphics->pEntityWorld);




		mpSwapchain = pResources->CreateSwapchain(pGraphics->pPrimaryDevice, *pGraphics->pSurface, 3);		
		mpSwapTimer = new VulkanSwapchainTimer(mpSwapchain);

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

		VulkanRenderpassInfo renderpassInfo = {};
		renderpassInfo.attachments =
		{
			{ "Swapchain",		VULKAN_ATTACHMENT_TYPE_SWAPCHAIN,		VK_FORMAT_B8G8R8A8_UNORM },
			{ "Depth-Stencil",	VULKAN_ATTACHMENT_TYPE_DEPTH_STENCIL,	VK_FORMAT_D24_UNORM_S8_UINT }
		};
		renderpassInfo.subpasses =
		{
			{ "Color-Subpass", { 0, 1 } }
		};

		VulkanRenderpass* pRenderPass = pResources->CreateRenderpass(pGraphics->pPrimaryDevice, renderpassInfo);

		VulkanGraphicsPipelineInfo pipelineInfo = {};
		pipelineInfo.shaders				= { pVertexShader, pFragmentShader };
		pipelineInfo.dynamicViewport		= false;
		pipelineInfo.viewport.x				= 0;
		pipelineInfo.viewport.y				= pGraphics->pSurface->height;
		pipelineInfo.viewport.width			= pGraphics->pSurface->width;
		pipelineInfo.viewport.height		= -(float)pGraphics->pSurface->height;
		pipelineInfo.viewport.minDepth		= 0.0f;
		pipelineInfo.viewport.maxDepth		= 1.0f;
		pipelineInfo.scissor.offset.x		= 0;
		pipelineInfo.scissor.offset.y		= 0;
		pipelineInfo.scissor.extent.width	= pGraphics->pSurface->width;
		pipelineInfo.scissor.extent.height	= pGraphics->pSurface->height;
		pipelineInfo.vkTopology				= VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		pipelineInfo.vkPolygonMode			= VK_POLYGON_MODE_FILL;
		pipelineInfo.vkCullMode				= VK_CULL_MODE_BACK_BIT;
		pipelineInfo.vkFrontFace			= VK_FRONT_FACE_CLOCKWISE;
		pipelineInfo.lineWidth				= 1.0f;
		pipelineInfo.multisamples			= VK_SAMPLE_COUNT_1_BIT;
		pipelineInfo.depth.enableTesting	= true;
		pipelineInfo.depth.enableWrite		= true;
		pipelineInfo.depth.compareOp		= VK_COMPARE_OP_LESS_OR_EQUAL;
		pipelineInfo.depth.depthMin			= 0.0f;
		pipelineInfo.depth.depthMax			= 1.0f;
		pipelineInfo.stencil.enableTesting	= false;
		pipelineInfo.stencil.compareOp		= VK_COMPARE_OP_LESS_OR_EQUAL;
		pipelineInfo.usePushDescriptors		= true;

		VkVertexInputBindingDescription vertexBufferAttachment = {};
		vertexBufferAttachment.binding		= 0;
		vertexBufferAttachment.stride		= 6 * sizeof(float);
		vertexBufferAttachment.inputRate	= VK_VERTEX_INPUT_RATE_VERTEX;

		pipelineInfo.bufferAttachments.PushBack(vertexBufferAttachment);

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

		pipelineInfo.vertexAttributes.PushBack(positionAttrib);
		pipelineInfo.vertexAttributes.PushBack(normalAttrib);
		//pipelineInfo.vertexAttributes.PushBack(tangentAttrib);
		//pipelineInfo.vertexAttributes.PushBack(texCoordAttrib);

		pipelineInfo.pRenderpass			= pRenderPass;

		VkPipelineColorBlendAttachmentState	blendAttachment = {};
		blendAttachment.blendEnable			= VK_TRUE;
		blendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
		blendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
		blendAttachment.colorBlendOp		= VK_BLEND_OP_ADD;
		blendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
		blendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
		blendAttachment.alphaBlendOp		= VK_BLEND_OP_ADD;
		blendAttachment.colorWriteMask		= VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;

		pipelineInfo.blendAttachments.PushBack(blendAttachment);

		mpPipeline = pResources->CreateGraphicsPipeline(pGraphics->pPrimaryDevice, pipelineInfo, 0);


		// Framebuffers

		for (uSize i = 0; i < 3; i++)
		{
			VulkanImageInfo depthImageInfo = {};
			depthImageInfo.vkFormat			= VK_FORMAT_D24_UNORM_S8_UINT;
			depthImageInfo.vkImageType		= VK_IMAGE_TYPE_2D;
			depthImageInfo.vkUsageFlags		= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
			depthImageInfo.width			= pGraphics->pSurface->width;
			depthImageInfo.height			= pGraphics->pSurface->height;
			depthImageInfo.depth			= 1;
			depthImageInfo.layers			= 1;
			depthImageInfo.mips				= 1;

			VulkanImage* pDepthStencilImage = pResources->CreateImage(pGraphics->pPrimaryDevice, depthImageInfo);

			VulkanImageViewInfo depthImageViewInfo = {};
			depthImageViewInfo.pImage			= pDepthStencilImage;
			depthImageViewInfo.vkImageViewType	= VK_IMAGE_VIEW_TYPE_2D;
			depthImageViewInfo.vkFormat			= VK_FORMAT_D24_UNORM_S8_UINT;
			depthImageViewInfo.vkAspectFlags	= VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
			depthImageViewInfo.layerStart		= 0;
			depthImageViewInfo.layerCount		= 1;
			depthImageViewInfo.mipStart			= 0;
			depthImageViewInfo.mipCount			= 1;

			VulkanImageView* pDepthStencilImageView = pResources->CreateImageView(pGraphics->pPrimaryDevice, depthImageViewInfo);

			VulkanFramebufferInfo framebufferInfo = {};
			framebufferInfo.renderpass	= pRenderPass;
			framebufferInfo.attachments = { mpSwapTimer->GetSwapchain()->imageViews[i], pDepthStencilImageView };
			framebufferInfo.width		= pGraphics->pSurface->width;
			framebufferInfo.height		= pGraphics->pSurface->height;
			framebufferInfo.layers		= 1;

			mFramebuffers[i] = pResources->CreateFramebuffer(pGraphics->pPrimaryDevice, framebufferInfo);
		}


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
		
			VulkanRenderpassBeginInfo renderpassBeginInfo = {};
			renderpassBeginInfo.pFramebuffer	= mFramebuffers[i];
			renderpassBeginInfo.renderArea		= { { 0, 0 }, {mFramebuffers[i]->width, mFramebuffers[i]->height}};
			renderpassBeginInfo.clearValues		= { { 0.02f, 0.05f, 0.05f, 1.0f }, { 1.0f, 0 } };

			renderRecorder.CopyBuffer(mUniformTransformStagingBuffers[i], mUniformTransformBuffers[i], transformUniformBufferSize, 0, 0);

			renderRecorder.BeginRenderpass(pRenderPass, renderpassBeginInfo);

			renderRecorder.SetVertexBuffers({{pVertexBuffer, 0}});
			renderRecorder.SetIndexBuffer(pIndexBuffer, 0, VK_INDEX_TYPE_UINT16);
			renderRecorder.SetGraphicsPipeline(mpPipeline);

			VulkanUniformBinding binding = {};
			binding.binding = 0;
			binding.pBuffer = mUniformTransformBuffers[i];
			binding.offset	= 0;
			binding.range	= mUniformTransformBuffers[i]->sizeBytes;

			renderRecorder.BindUniforms(mpPipeline, 0, { binding });

			renderRecorder.DrawIndexed(1, pIndexBuffer->sizeBytes / sizeof(uInt16), 0);

			renderScene.RecordRender(&renderRecorder, mpPipeline);

			renderRecorder.EndRenderpass();

			renderRecorder.EndRecording();
		}
	}

	void VulkanRenderer::RenderScene(VulkanRenderScene* pRenderScene)
	{
		mpSwapTimer->AdvanceFrame();

		uInt32 resourceIdx = mpSwapTimer->GetFrameIndex();

		VulkanSubmission renderSubmition	= {};
		renderSubmition.commandBuffers		= { mCommandBuffers[resourceIdx] };
		renderSubmition.waitSemaphores		= { mpSwapTimer->GetCurrentAcquiredSemaphore() };
		renderSubmition.waitStages			= { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
		renderSubmition.signalSemaphores	= { mpSwapTimer->GetCurrentCompleteSemaphore() };

		mpGraphics->Submit(renderSubmition, mpGraphics->pPrimaryDevice->queues.graphics, mpSwapTimer->GetCurrentFence());

		mpSwapTimer->Present();
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