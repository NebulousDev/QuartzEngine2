#include "Vulkan/VulkanFrameGraph.h"

#include "Vulkan/Primatives/VulkanImage.h"
#include "Vulkan/Primatives/VulkanBuffer.h"
#include "Vulkan/Primatives/VulkanSurface.h"
#include "Vulkan/VulkanCommandRecorder.h"

#include "Application.h"

//#ifdef WIN32
//#define WIN32_LEAN_AND_MEAN
//#include <Windows.h>
//#include <Vulkan/vulkan_win32.h>
//#endif

namespace Quartz
{
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

	VulkanImage* VulkanFrameGraph::GetActiveImage(FrameGraphImage& graphImage)
	{
		if (graphImage.flags & RESOURCE_FLAG_WINDOW_OUTPUT)
		{
			VulkanSwapchainTimer* pSwapchainTimer = nullptr;

			auto& swapchainIt = mOutputSwapchains.Find(graphImage.id);
			if (swapchainIt != mOutputSwapchains.End())
			{
				pSwapchainTimer = &swapchainIt->value;
				VulkanImage* pImage = pSwapchainTimer->GetCurrentImage();
				return pImage;
			}

			return nullptr;
		}
		else
		{
			auto& imageIt = mImages.Find(graphImage.id);
			if (imageIt != mImages.End())
			{
				VulkanImage* pImage = imageIt->value[mBackbufferIdx];
				return pImage;
			}

			return nullptr;
		}
	}

	VulkanBuffer* VulkanFrameGraph::GetActiveBuffer(FrameGraphBuffer& graphBuffer)
	{
		// @TODO
		return nullptr;
	}

	void VulkanFrameGraph::TransitionImage(
		VulkanCommandRecorder& commandRecorder,
		const FrameGraphImageTransition& oldState,
		const FrameGraphImageTransition& newState)
	{
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

		VulkanImage* pImage = GetActiveImage(*newState.pImage);

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

		commandRecorder.PipelineBarrier(barrierInfo);
	};

	void VulkanFrameGraph::PassTransitionImages(VulkanCommandRecorder& recorder, const FrameGraphPass& pass)
	{
		for (const FrameGraphImageTransition& imageTransition : pass.GetImageTransitions())
		{
			const FrameGraphImageTransition& oldState = imageTransition.pImage->transitionState;
			const FrameGraphImageTransition& newState = imageTransition;

			TransitionImage(recorder, oldState, newState);
		}
	}

	bool VulkanFrameGraph::ApiInitialize(GraphicsApiInfo& info)
	{
		VulkanGraphics* pVulkanGraphics = (VulkanGraphics*)info.pNativeApi;
		mpGraphics			= pVulkanGraphics;
		mpResourceManager	= &pVulkanGraphics->resourceManager;
		mpDevice			= pVulkanGraphics->pPrimaryDevice;

		VulkanCommandPoolInfo graphicsPoolInfo = {};
		graphicsPoolInfo.queueFamilyIndex			= mpDevice->pPhysicalDevice->primaryQueueFamilyIndices.graphics;
		graphicsPoolInfo.vkCommandPoolCreateFlags	= VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

		mpGraphicsPool	= mpResourceManager->CreateCommandPool(mpDevice, graphicsPoolInfo);
		mpComputePool	= mpGraphicsPool;
		mpTransferPool	= mpGraphicsPool;

		constexpr uSize backbufferCount = 3;

		if (backbufferCount < 1)
		{
			mBackbufferCount = 1;
		}
		else if (backbufferCount > 3)
		{
			mBackbufferCount = 3;
		}

		mBackbufferCount = backbufferCount;

		mRecorders.Resize(mBackbufferCount);
		mCommandBuffers.Resize(mBackbufferCount);

		mpResourceManager->CreateCommandBuffers(mpGraphicsPool, mBackbufferCount, mCommandBuffers.Data());

		for (uSize i = 0; i < mBackbufferCount; i++)
		{
			mRecorders[i] = VulkanCommandRecorder(mCommandBuffers[i]);
		}

		return true;
	}

	bool VulkanFrameGraph::ApiDestroy()
	{
		// Destroy Pools
		return true;
	}

	ApiImageHandle VulkanFrameGraph::ApiAquireImage(const FrameGraphImage& graphImage)
	{
		if (mImages.Contains(graphImage.id))
		{
			return ApiImageHandle(graphImage.id);
		}

		if (graphImage.flags & RESOURCE_FLAG_WINDOW_OUTPUT)
		{
			// Swapchain image

			VkSurfaceKHR	vkSurface;
			Window*			pWindow;

			auto& windowIt = mWindowOutputs.Find(graphImage.id);
			if (windowIt != mWindowOutputs.End())
			{
				pWindow = windowIt->value;
			}
			else
			{
				assert(false && "Graph image marked as RESOURCE_FLAG_WINDOW_OUTPUT but no window was found.");
				return ApiImageHandle(0);
			}

			VkInstance vkInstance = mpGraphics->vkInstance;
			VulkanApiSurface* pApiSurface = (VulkanApiSurface*)pWindow->GetSurface();
			VulkanSurface* pSurface = mpResourceManager->CreateSurface(mpDevice, vkInstance, *pApiSurface);

			VulkanSwapchain* pSwapchain = mpResourceManager->CreateSwapchain(mpDevice, *pSurface, mBackbufferCount);

			mOutputSurfaces.Put(graphImage.id, pSurface);
			mOutputSwapchains.Put(graphImage.id, VulkanSwapchainTimer(pSwapchain));
		}
		else
		{
			// Transient or permanant image

			VulkanImageInfo imageInfo = {};
			imageInfo.width		= graphImage.info.width;
			imageInfo.height	= graphImage.info.height;
			imageInfo.depth		= graphImage.info.depth;
			imageInfo.layers	= graphImage.info.layers;
			imageInfo.mips		= graphImage.info.mipLevels;

			switch (graphImage.info.format)
			{
				case IMAGE_FORMAT_R8:		imageInfo.vkFormat = VK_FORMAT_R8_SNORM;			break;
				case IMAGE_FORMAT_R8G8:		imageInfo.vkFormat = VK_FORMAT_R8G8_SNORM;			break;
				case IMAGE_FORMAT_R8G8B8:	imageInfo.vkFormat = VK_FORMAT_R8G8B8_SNORM;		break;
				case IMAGE_FORMAT_R8G8B8A8:	imageInfo.vkFormat = VK_FORMAT_R8G8B8A8_SNORM;		break;
				case IMAGE_FORMAT_D32:		imageInfo.vkFormat = VK_FORMAT_D32_SFLOAT;			break;
				case IMAGE_FORMAT_D24S8:	imageInfo.vkFormat = VK_FORMAT_D24_UNORM_S8_UINT;	break;

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

			if (graphImage.flags & RESOURCE_FLAG_BACKBUFFER)
			{
				Array<VulkanImage*, 3>& images = mImages.Put(graphImage.id, Array<VulkanImage*, 3>(3));

				for (uSize i = 0; i < mBackbufferCount; i++)
				{
					images[i] = mpResourceManager->CreateImage(mpDevice, imageInfo);
				}
			}
			else
			{
				Array<VulkanImage*, 3>& images = mImages.Put(graphImage.id, Array<VulkanImage*, 3>(1));
				images[0] = mpResourceManager->CreateImage(mpDevice, imageInfo);
			}
		}

		return ApiImageHandle(graphImage.id);
	}

	void VulkanFrameGraph::ApiReleaseImage(const FrameGraphImage& graphImage)
	{
		auto& viewIt = mImageViews.Find(graphImage.id);
		if (viewIt == mImageViews.End())
		{
			return; // Image view not found
		}

		for (VulkanImageView* pView : viewIt->value)
		{
			mpResourceManager->DestroyImageView(pView);
		}

		mImageViews.Remove(graphImage.id);

		auto& imageIt = mImages.Find(graphImage.id);
		if (imageIt == mImages.End())
		{
			return; // Image not found
		}

		for (VulkanImage* pImage : imageIt->value)
		{
			mpResourceManager->DestroyImage(pImage);
		}

		mImages.Remove(graphImage.id);
	}

	ApiBufferHandle VulkanFrameGraph::ApiAquireBuffer(const FrameGraphBuffer& graphBuffer)
	{
		if (mBuffers.Contains(graphBuffer.id))
		{
			return ApiBufferHandle(graphBuffer.id);
		}

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

		if (graphBuffer.flags & RESOURCE_FLAG_BACKBUFFER)
		{
			Array<VulkanBuffer*, 3>& buffers = mBuffers.Put(graphBuffer.id, Array<VulkanBuffer*, 3>(3));

			for (uSize i = 0; i < mBackbufferCount; i++)
			{
				buffers[i] = mpResourceManager->CreateBuffer(mpDevice, bufferInfo);
			}
		}
		else
		{
			Array<VulkanBuffer*, 3>& buffers = mBuffers.Put(graphBuffer.id, Array<VulkanBuffer*, 3>(1));
			buffers[0] = mpResourceManager->CreateBuffer(mpDevice, bufferInfo);
		}

		return ApiBufferHandle(graphBuffer.id);
	}

	void VulkanFrameGraph::ApiReleaseBuffer(const FrameGraphBuffer& graphBuffer)
	{
		// @TODO
	}

	ApiRecorderHandle VulkanFrameGraph::ApiAquireRecorder(const FrameGraphCommandRecorder& graphRecorder)
	{
		return nullptr; // @TODO: Properly implement generic recorders
	}

	void VulkanFrameGraph::ApiReleaseRecorder(const FrameGraphCommandRecorder& graphRecorder)
	{

	}

	void VulkanFrameGraph::ApiExecuteFrame()
	{
		VulkanCommandRecorder& recorder = mRecorders[mBackbufferIdx];

		for (auto& swapchainPair : mOutputSwapchains)
		{
			VulkanSwapchainTimer& swapchainTimer = swapchainPair.value;
			swapchainTimer.AdvanceFrame();
		}

		recorder.Reset();

		recorder.BeginRecording();

		for (uSize i = 0; i < mOrderedPasses.Size(); i++)
		{
			const FrameGraphPass& pass = mPasses[mOrderedPasses[i]];

			PassTransitionImages(recorder, pass);
			pass.Execute(mCommandRecorder);
		}

		for (auto& swapchainPair : mOutputSwapchains)
		{
			VulkanSwapchainTimer& swapchainTimer = swapchainPair.value;
			recorder.PipelineBarrierSwapchainImageEnd(swapchainTimer.GetCurrentImage());
		}

		recorder.EndRecording();

		for (auto& swapchainPair : mOutputSwapchains)
		{
			VulkanSwapchainTimer& swapchainTimer = swapchainPair.value;

			VulkanSubmission renderSubmition	= {};
			renderSubmition.commandBuffers		= { &recorder.GetCommandBuffer() };
			renderSubmition.waitSemaphores		= { swapchainTimer.GetCurrentAcquiredSemaphore() };
			renderSubmition.waitStages			= { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
			renderSubmition.signalSemaphores	= { swapchainTimer.GetCurrentCompleteSemaphore() };

			mpGraphics->Submit(renderSubmition, mpGraphics->pPrimaryDevice->queues.graphics, swapchainTimer.GetCurrentFence());

			swapchainTimer.Present();
		}

		mBackbufferIdx = (mBackbufferIdx + 1) % mBackbufferCount;
	}

	VulkanFrameGraph::VulkanFrameGraph(): 
		mpGraphics(nullptr),
		mpResourceManager(nullptr),
		mpDevice(nullptr),
		mpGraphicsPool(nullptr),
		mpComputePool(nullptr),
		mpTransferPool(nullptr),
		mBackbufferCount(1),
		mBackbufferIdx(0) { }

	VulkanImageView* VulkanFrameGraph::GetActiveImageView(const WrapperString& imageName)
	{
		FrameGraphImage* pGraphImage = GetImageByName(imageName);

		if (!pGraphImage)
		{
			return nullptr;
		}

		auto& viewIt = mImageViews.Find(pGraphImage->id);
		if (viewIt != mImageViews.End())
		{
			return viewIt->value[mBackbufferIdx];
		}

		if (pGraphImage->flags & RESOURCE_FLAG_WINDOW_OUTPUT)
		{
			auto& swapchainIt = mOutputSwapchains.Find(pGraphImage->id);
			if (swapchainIt != mOutputSwapchains.End())
			{
				VulkanSwapchainTimer& swapchainTimer = swapchainIt->value;

				Array<VulkanImageView*, 3>& views = mImageViews.Put(pGraphImage->id);
				views.Resize(mBackbufferCount);

				for (uSize i = 0; i < mBackbufferCount; i++)
				{
					views[i] = swapchainTimer.GetSwapchain()->imageViews[i];
				}

				return views[mBackbufferIdx];
			}
		}
		else
		{
			Array<VulkanImage*, 3>* pImages = nullptr;

			auto& imagesIt = mImages.Find(pGraphImage->id);
			if (imagesIt != mImages.End())
			{
				pImages = &imagesIt->value;
			}
			else
			{
				return nullptr;
			}
			
			Array<VulkanImageView*, 3>& views = mImageViews.Put(pGraphImage->id);
			views.Resize(mBackbufferCount);

			for (uSize i = 0; i < mBackbufferCount; i++)
			{
				VulkanImage* pImage = (*pImages)[i];

				VulkanImageViewInfo viewInfo = {};
				viewInfo.pImage				= pImage;
				viewInfo.vkFormat			= pImage->vkFormat;
				viewInfo.vkImageViewType	= VK_IMAGE_VIEW_TYPE_2D;

				if (pGraphImage->usages & IMAGE_USAGE_DEPTH_INPUT ||
					pGraphImage->usages & IMAGE_USAGE_DEPTH_OUTPUT)
				{
					viewInfo.vkAspectFlags	= VK_IMAGE_ASPECT_DEPTH_BIT;
				}
				else if (pGraphImage->usages & IMAGE_USAGE_DEPTH_STENCIL_INPUT ||
					pGraphImage->usages & IMAGE_USAGE_DEPTH_STENCIL_OUTPUT)
				{
					viewInfo.vkAspectFlags = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
				}
				else
				{
					viewInfo.vkAspectFlags = VK_IMAGE_ASPECT_COLOR_BIT;
				}

				viewInfo.layerStart			= 0;
				viewInfo.layerCount			= pImage->layers;
				viewInfo.mipStart			= 0;
				viewInfo.mipCount			= pImage->mips;

				views[i] = mpResourceManager->CreateImageView(mpDevice, viewInfo);
			}

			return views[mBackbufferIdx];
		}

		return nullptr;
	}

	VulkanCommandRecorder& VulkanFrameGraph::GetActiveRecorder()
	{
		return mRecorders[mBackbufferIdx];
	}
}