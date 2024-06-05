#include "Vulkan/VulkanFrameGraph.h"

#include "Vulkan/Primatives/VulkanImage.h"
#include "Vulkan/Primatives/VulkanBuffer.h"
#include "Vulkan/VulkanCommandRecorder.h"

namespace Quartz
{
	VulkanFrameGraphState gVulkanFrameGraphState = {};

	VkPipelineStageFlagBits GetVkPipelineStage(ShaderStage shaderStage)
	{
		switch (shaderStage)
		{
			case SHADER_STAGE_VERTEX:					return VK_PIPELINE_STAGE_VERTEX_SHADER_BIT;
			case SHADER_STAGE_TESSELLATION_CONTROL:		return VK_PIPELINE_STAGE_TESSELLATION_CONTROL_SHADER_BIT;
			case SHADER_STAGE_TESSELLATION_EVALUATION:	return VK_PIPELINE_STAGE_TESSELLATION_EVALUATION_SHADER_BIT;
			case SHADER_STAGE_GEOMETRY:					return VK_PIPELINE_STAGE_GEOMETRY_SHADER_BIT;
			case SHADER_STAGE_FRAGMENT:					return VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
			case SHADER_STAGE_COMPUTE:					return VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
			case SHADER_STAGE_TASK:						return VK_PIPELINE_STAGE_TASK_SHADER_BIT_EXT;
			case SHADER_STAGE_MESH:						return VK_PIPELINE_STAGE_MESH_SHADER_BIT_EXT;
			case SHADER_STAGE_RAY_GENERATION:			return VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR;
			case SHADER_STAGE_INTERSECTION:				return VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR;
			case SHADER_STAGE_ANY_HIT:					return VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR;
			case SHADER_STAGE_CLOSEST_HIT:				return VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR;
			case SHADER_STAGE_MISS:						return VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR;
			case SHADER_STAGE_CALLABLE:					return VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT; // ???
			case SHADER_STAGE_TOP_OF_PIPE:				return VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
			case SHADER_STAGE_BOTTOM_OF_PIPE:			return VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
		}

		LogError("Error in GetVkPipelineStage(): Enum out of bounds.");

		return VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
	}

	void SetupVulkanFrameGraph(VulkanGraphics& graphics)
	{
		VulkanCommandPoolInfo graphicsPoolInfo = {};
		graphicsPoolInfo.queueFamilyIndex			= graphics.pPrimaryDevice->pPhysicalDevice->primaryQueueFamilyIndices.graphics;
		graphicsPoolInfo.vkCommandPoolCreateFlags	= 0;

		gVulkanFrameGraphState.pGraphicsPool	= graphics.pResourceManager->CreateCommandPool(graphics.pPrimaryDevice, graphicsPoolInfo);
		gVulkanFrameGraphState.pComputePool		= gVulkanFrameGraphState.pGraphicsPool;
		gVulkanFrameGraphState.pTransferPool	= gVulkanFrameGraphState.pGraphicsPool;
	}

	void SetupVulkanFrameGraphFunctions(VulkanGraphics& graphics, FrameGraph::FrameGraphFunctions& outFunctions)
	{
		outFunctions.createPhysicalImageFunc =
			[&graphics](const FrameGraphImage& graphImage) -> PhysicalImageHandle
			{
				VulkanImageInfo imageInfo = {};
				imageInfo.width		= graphImage.info.width;
				imageInfo.height	= graphImage.info.height;
				imageInfo.depth		= graphImage.info.depth;
				imageInfo.layers	= graphImage.info.layers;
				imageInfo.mips		= graphImage.info.mipLevels;

				switch (graphImage.info.format)
				{
					case IMAGE_FORMAT_R8:		imageInfo.vkFormat = VK_FORMAT_R8_SNORM;		break;
					case IMAGE_FORMAT_R8G8:		imageInfo.vkFormat = VK_FORMAT_R8G8_SNORM;		break;
					case IMAGE_FORMAT_R8G8B8:	imageInfo.vkFormat = VK_FORMAT_R8G8B8_SNORM;	break;
					case IMAGE_FORMAT_R8G8B8A8:	imageInfo.vkFormat = VK_FORMAT_R8G8B8A8_SNORM;	break;

					default:
					{
						imageInfo.vkFormat = VK_FORMAT_R8G8B8A8_SNORM;
						LogError("Error creating vulkan Frame Graph image: Invalid image format enum.");
					}
				};

				switch (graphImage.info.type)
				{
					case IMAGE_TYPE_1D:	imageInfo.vkImageType = VK_IMAGE_TYPE_1D; break;
					case IMAGE_TYPE_2D:	imageInfo.vkImageType = VK_IMAGE_TYPE_2D; break;
					case IMAGE_TYPE_3D:	imageInfo.vkImageType = VK_IMAGE_TYPE_3D; break;

					default:
					{
						imageInfo.vkImageType = VK_IMAGE_TYPE_2D;
						LogError("Error creating vulkan Frame Graph image: Invalid image type enum.");
					}
				}

				if (graphImage.usages & IMAGE_USAGE_COLOR_INPUT)
				{
					imageInfo.vkUsageFlags |= VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT;
				}
				if (graphImage.usages & IMAGE_USAGE_COLOR_OUTPUT)
				{
					imageInfo.vkUsageFlags |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
				}
				if (graphImage.usages & IMAGE_USAGE_DEPTH_INPUT)
				{
					imageInfo.vkUsageFlags |= VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT;
				}
				if (graphImage.usages & IMAGE_USAGE_DEPTH_OUTPUT)
				{
					imageInfo.vkUsageFlags |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
				}
				if (graphImage.usages & IMAGE_USAGE_DEPTH_STENCIL_INPUT)
				{
					imageInfo.vkUsageFlags |= VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT;
				}
				if (graphImage.usages & IMAGE_USAGE_DEPTH_STENCIL_OUTPUT)
				{
					imageInfo.vkUsageFlags |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
				}
				if (graphImage.usages & IMAGE_USAGE_SAMPLED_TEXUTRE)
				{
					imageInfo.vkUsageFlags |= VK_IMAGE_USAGE_SAMPLED_BIT;
				}
				if (graphImage.usages & IMAGE_USAGE_TRANSFER_SOURCE)
				{
					imageInfo.vkUsageFlags |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
				}
				if (graphImage.usages & IMAGE_USAGE_TRANSFER_DESTINATION)
				{
					imageInfo.vkUsageFlags |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
				}
				
				if(imageInfo.vkUsageFlags == 0)
				{
					LogWarning("Warning creating vulkan Frame Graph image: No image usage assigned.");
				}

				VulkanImage* pVulkanImage = graphics.pResourceManager->CreateImage(graphics.pPrimaryDevice, imageInfo);

				return PhysicalImageHandle(pVulkanImage);
			};

		outFunctions.createPhysicalBufferFunc =
			[&graphics](const FrameGraphBuffer& graphBuffer) -> PhysicalImageHandle
			{
				VulkanBufferInfo bufferInfo = {};
				bufferInfo.sizeBytes = graphBuffer.info.sizeBytes;

				if (graphBuffer.usages & BUFFER_USAGE_VERTEX_INPUT)
				{
					bufferInfo.vkUsageFlags |= VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
				}
				if (graphBuffer.usages & BUFFER_USAGE_INDEX_INPUT)
				{
					bufferInfo.vkUsageFlags |= VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
				}
				if (graphBuffer.usages & BUFFER_USAGE_UNIFORM_INPUT)
				{
					bufferInfo.vkUsageFlags |= VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
				}
				if (graphBuffer.usages & BUFFER_USAGE_TRANSFER_SOURCE)
				{
					bufferInfo.vkUsageFlags |= VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
				}
				if (graphBuffer.usages & BUFFER_USAGE_TRANSFER_DESTINATION)
				{
					bufferInfo.vkUsageFlags |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;
				}
				
				if(bufferInfo.vkUsageFlags == 0)
				{
					LogWarning("Warning creating vulkan Frame Graph buffer: No buffer usage assigned.");
				}

				// @TODO
				bufferInfo.vkMemoryProperties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

				VulkanBuffer* pVulkanBuffer = graphics.pResourceManager->CreateBuffer(graphics.pPrimaryDevice, bufferInfo);

				return PhysicalBufferHandle(pVulkanBuffer);
			};

		outFunctions.createPhysicalCommandBufferFunc =
			[&graphics](const FrameGraphCommandBuffer& graphCommandBuffer) -> PhysicalImageHandle
			{
				VulkanCommandPool* pCommandPool = nullptr;

				if (graphCommandBuffer.capableQueues & QUEUE_TRANSFER)
				{
					pCommandPool = gVulkanFrameGraphState.pTransferPool;
				}

				if (graphCommandBuffer.capableQueues & QUEUE_COMPUTE)
				{
					pCommandPool = gVulkanFrameGraphState.pComputePool;
				}

				if (graphCommandBuffer.capableQueues & QUEUE_GRAPHICS ||
					graphCommandBuffer.capableQueues & QUEUE_PRESENT)
				{
					pCommandPool = gVulkanFrameGraphState.pGraphicsPool;
				}

				VulkanCommandBuffer* pVulkanCommandBuffer;
				graphics.pResourceManager->CreateCommandBuffers(pCommandPool, 1, &pVulkanCommandBuffer);

				// @TODO destroy
				VulkanCommandRecorder* pRecorder = new VulkanCommandRecorder(pVulkanCommandBuffer);

				return PhysicalBufferHandle(pRecorder);
			};

		outFunctions.beginCommandBufferFunc =
			[&graphics](const FrameGraphCommandBuffer& graphCommandBuffer)
			{
				VulkanCommandRecorder* pRecorder = (VulkanCommandRecorder*)graphCommandBuffer.hPhysicalCommandBuffer;
				pRecorder->BeginRecording();
			};

		outFunctions.endCommandBufferFunc =
			[&graphics](const FrameGraphCommandBuffer& graphCommandBuffer)
			{
				VulkanCommandRecorder* pRecorder = (VulkanCommandRecorder*)graphCommandBuffer.hPhysicalCommandBuffer;
				pRecorder->EndRecording();
			};

		outFunctions.submitCommandBufferFunc =
			[&graphics](const FrameGraphCommandBuffer& graphCommandBuffer)
			{
				VulkanCommandRecorder* pRecorder = (VulkanCommandRecorder*)graphCommandBuffer.hPhysicalCommandBuffer;
				
				//graphics.Submit()
			};

		outFunctions.transitionImageFunc =
			[&graphics](FrameGraphCommandBuffer& graphCommandBuffer,
				const FrameGraphImageTransition& oldState,
				const FrameGraphImageTransition& newState)
			{
				VulkanCommandRecorder* pRecorder	= (VulkanCommandRecorder*)graphCommandBuffer.hPhysicalCommandBuffer;
				VulkanImage* pImage					= (VulkanImage*)oldState.pImage->hPhysicalImage;

				VkAccessFlags			srcAccess	= 0;
				VkAccessFlags			destAccess	= 0;
				VkImageAspectFlags		aspectMask	= 0;
				VkImageLayout			srcLayout	= VK_IMAGE_LAYOUT_UNDEFINED;
				VkImageLayout			destLayout	= VK_IMAGE_LAYOUT_UNDEFINED;
				VkPipelineStageFlagBits	srcStage	= VK_PIPELINE_STAGE_NONE;
				VkPipelineStageFlagBits	destStage	= VK_PIPELINE_STAGE_NONE;

				// @TODO: Read + write access??

				switch (oldState.usage)
				{
					case IMAGE_USAGE_NONE:					
						srcLayout = VK_IMAGE_LAYOUT_UNDEFINED;
						srcAccess = VK_ACCESS_NONE;
						srcStage  = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
						break;
					case IMAGE_USAGE_COLOR_INPUT:			
						srcLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;		
						srcAccess = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;
						srcStage  = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
						break;
					case IMAGE_USAGE_COLOR_OUTPUT:			
						srcLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;		
						srcAccess = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
						srcStage  = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
						break;
					case IMAGE_USAGE_DEPTH_INPUT:			
						srcLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;		
						srcAccess = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
						srcStage  = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
						break;
					case IMAGE_USAGE_DEPTH_OUTPUT:			
						srcLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;		
						srcAccess = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
						srcStage  = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
						break;
					case IMAGE_USAGE_DEPTH_STENCIL_INPUT:	
						srcLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;	
						srcAccess = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
						srcStage  = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
						break;
					case IMAGE_USAGE_DEPTH_STENCIL_OUTPUT:	
						srcLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;	
						srcAccess = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
						srcStage  = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
						break;
					case IMAGE_USAGE_SAMPLED_TEXUTRE:		
						srcLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;			
						srcAccess = VK_ACCESS_SHADER_READ_BIT;
						srcStage  = GetVkPipelineStage(oldState.stage);
						break;
					case IMAGE_USAGE_TRANSFER_SOURCE:		
						srcLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;			
						srcAccess = VK_ACCESS_TRANSFER_READ_BIT;
						srcStage  = VK_PIPELINE_STAGE_TRANSFER_BIT;
						break;
					case IMAGE_USAGE_TRANSFER_DESTINATION:	
						srcLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;			
						srcAccess = VK_ACCESS_TRANSFER_WRITE_BIT;
						srcStage  = VK_PIPELINE_STAGE_TRANSFER_BIT;
						break;
				}

				switch (newState.usage)
				{
					case IMAGE_USAGE_NONE:				
						destLayout = VK_IMAGE_LAYOUT_UNDEFINED;							
						destAccess = VK_ACCESS_NONE;
						aspectMask = VK_IMAGE_ASPECT_NONE;
						destStage  = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
						break;
					case IMAGE_USAGE_COLOR_INPUT:			
						destLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;		
						destAccess = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;
						aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
						destStage  = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
						break;
					case IMAGE_USAGE_COLOR_OUTPUT:			
						destLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;		
						destAccess = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
						aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
						destStage  = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
						break;
					case IMAGE_USAGE_DEPTH_INPUT:			
						destLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;			
						destAccess = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
						aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
						destStage  = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
						break;
					case IMAGE_USAGE_DEPTH_OUTPUT:			
						destLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;			
						destAccess = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
						aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
						destStage  = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
						break;
					case IMAGE_USAGE_DEPTH_STENCIL_INPUT:	
						destLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;	
						destAccess = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
						aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
						destStage  = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
						break;
					case IMAGE_USAGE_DEPTH_STENCIL_OUTPUT:	
						destLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;	
						destAccess = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
						aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
						destStage  = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
						break;
					case IMAGE_USAGE_SAMPLED_TEXUTRE:		
						destLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;			
						destAccess = VK_ACCESS_SHADER_READ_BIT;
						aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
						destStage  = GetVkPipelineStage(newState.stage);
						break;
					case IMAGE_USAGE_TRANSFER_SOURCE:		
						destLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;			
						destAccess = VK_ACCESS_TRANSFER_READ_BIT;
						aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
						destStage  = VK_PIPELINE_STAGE_TRANSFER_BIT;
						break;
					case IMAGE_USAGE_TRANSFER_DESTINATION:	
						destLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;				
						destAccess = VK_ACCESS_TRANSFER_WRITE_BIT;
						aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
						destStage  = VK_PIPELINE_STAGE_TRANSFER_BIT;
						break;
				}

				VkImageMemoryBarrier vkImageMemoryBarrier = {};
				vkImageMemoryBarrier.sType								= VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
				vkImageMemoryBarrier.srcAccessMask						= srcAccess;
				vkImageMemoryBarrier.dstAccessMask						= destAccess;
				vkImageMemoryBarrier.oldLayout							= srcLayout;
				vkImageMemoryBarrier.newLayout							= destLayout;
				vkImageMemoryBarrier.image								= pImage->vkImage;
				vkImageMemoryBarrier.subresourceRange.aspectMask		= aspectMask;
				vkImageMemoryBarrier.subresourceRange.baseMipLevel		= 0;
				vkImageMemoryBarrier.subresourceRange.levelCount		= 1;
				vkImageMemoryBarrier.subresourceRange.baseArrayLayer	= 0;
				vkImageMemoryBarrier.subresourceRange.layerCount		= 1;

				VulkanPipelineBarrierInfo barrierInfo = {};
				barrierInfo.srcStage					= srcStage;
				barrierInfo.dstStage					= destStage;
				barrierInfo.dependencyFlags				= 0;
				barrierInfo.memoryBarrierCount			= 0;
				barrierInfo.pMemoryBarriers				= nullptr;
				barrierInfo.bufferMemoryBarrierCount	= 0;
				barrierInfo.pBufferMemoryBarriers		= nullptr;
				barrierInfo.imageMemoryBarrierCount		= 1;
				barrierInfo.pImageMemoryBarriers		= &vkImageMemoryBarrier;

				pRecorder->PipelineBarrier(barrierInfo);
			};
	}
}