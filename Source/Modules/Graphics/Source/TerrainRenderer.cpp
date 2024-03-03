#include "TerrainRenderer.h"

#include "Math/Math.h"
#include "Engine.h"

namespace Quartz
{
	template<>
	uSize Hash<Vec2i>(const Vec2i& value)
	{
		return Hash<uInt64>((uInt64)value.x + ((uInt64)value.y << 32));
	}

	ModelData VulkanTerrainRenderer::CreateTileMesh(uSize resolution)
	{
		ModelData data;

		data.vertices.Reserve((resolution + 1) * (resolution + 1) * 6);
		data.indices.Reserve(resolution * resolution * 6);

		for (uInt32 y = 0; y < resolution + 1; y++)
		{
			for (uInt32 x = 0; x < resolution + 1; x++)
			{
				// @TODO: optimize
				data.vertices.PushBack((float)x / resolution);
				data.vertices.PushBack(0.0f);
				data.vertices.PushBack((float)y / resolution);
			}
		}

		for (uInt32 y = 0; y < resolution; y++)
		{
			for (uInt32 x = 0; x < resolution; x++)
			{
				uInt16 downLeft		= x + (y + 0) * (resolution + 1);
				uInt16 downRight	= x + (y + 0) * (resolution + 1) + 1;
				uInt16 upLeft		= x + (y + 1) * (resolution + 1);
				uInt16 upRight		= x + (y + 1) * (resolution + 1) + 1;

				// @TODO: optimize
				data.indices.PushBack(downLeft);
				data.indices.PushBack(upLeft);
				data.indices.PushBack(upRight);
				data.indices.PushBack(downLeft);
				data.indices.PushBack(upRight);
				data.indices.PushBack(downRight);
			}
		}

		return data;
	}

	void VulkanTerrainRenderer::CreateLODs(uSize count, uSize closeResolution)
	{
		VulkanResourceManager& resources = *mpGraphics->pResourceManager;
		VulkanDevice& device = *mpGraphics->pPrimaryDevice;

		/* Terrain Staging Buffers */

		VulkanBufferInfo meshLODStagingVertexBufferInfo = {};
		meshLODStagingVertexBufferInfo.sizeBytes			= 0;
		meshLODStagingVertexBufferInfo.vkBufferUsage		= VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
		meshLODStagingVertexBufferInfo.vkMemoryProperties	= VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

		VulkanBufferInfo meshLODStagingIndexBufferInfo = {};
		meshLODStagingIndexBufferInfo.sizeBytes				= 0;
		meshLODStagingIndexBufferInfo.vkBufferUsage			= VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
		meshLODStagingIndexBufferInfo.vkMemoryProperties	= VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

		for (uSize i = 0; i < count; i++)
		{
			uSize lodVertexResolution = (closeResolution / (i + 1)) + 1;
			uSize lodIndexResolution = (closeResolution / (i + 1));
			meshLODStagingVertexBufferInfo.sizeBytes += (lodVertexResolution * lodVertexResolution) * sizeof(TerrainMeshVertex);
			meshLODStagingIndexBufferInfo.sizeBytes += (lodIndexResolution * lodIndexResolution) * sizeof(uInt16) * 6;
		}

		VulkanBuffer* pMeshLODStagingVertexBuffer = resources.CreateBuffer(&device, meshLODStagingVertexBufferInfo);
		VulkanBuffer* pMeshLODStagingIndexBuffer = resources.CreateBuffer(&device, meshLODStagingIndexBufferInfo);
		VulkanMultiBuffer meshLODStagingVertexBuffer(pMeshLODStagingVertexBuffer);
		VulkanMultiBuffer meshLODStagingIndexBuffer(pMeshLODStagingIndexBuffer);

		constexpr uSize MAX_LOADED_CHUNKS = 128;

		// TODO Move out of LODs
		VulkanBufferInfo perTileStagingBufferInfo = {};
		perTileStagingBufferInfo.sizeBytes			= sizeof(TerrainPerTileData) * MAX_LOADED_CHUNKS;
		perTileStagingBufferInfo.vkBufferUsage		= VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
		perTileStagingBufferInfo.vkMemoryProperties	= VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

		mpPerTileStagingBuffer = resources.CreateBuffer(&device, perTileStagingBufferInfo);
		mpPerTileWriter = VulkanBufferWriter(mpPerTileStagingBuffer);
		mpPerTileDatas = mpPerTileWriter.Map<TerrainPerTileData>();
		
		/* Terrain Buffers */

		VulkanBufferInfo meshLODVertexBufferInfo = {};
		meshLODVertexBufferInfo.sizeBytes			= meshLODStagingVertexBufferInfo.sizeBytes;
		meshLODVertexBufferInfo.vkBufferUsage		= VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
		meshLODVertexBufferInfo.vkMemoryProperties	= VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

		VulkanBufferInfo meshLODIndexBufferInfo = {};
		meshLODIndexBufferInfo.sizeBytes			= meshLODStagingIndexBufferInfo.sizeBytes;
		meshLODIndexBufferInfo.vkBufferUsage		= VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
		meshLODIndexBufferInfo.vkMemoryProperties	= VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

		mpLODVertexBuffer = resources.CreateBuffer(&device, meshLODVertexBufferInfo);
		mpLODIndexBuffer = resources.CreateBuffer(&device, meshLODIndexBufferInfo);

		VulkanBufferInfo perTileBufferInfo = {};
		perTileBufferInfo.sizeBytes				= perTileStagingBufferInfo.sizeBytes;
		perTileBufferInfo.vkBufferUsage			= VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
		perTileBufferInfo.vkMemoryProperties		= VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

		mpPerTileBuffer = resources.CreateBuffer(&device, perTileBufferInfo);

		/* Copy Data to Staging Buffers */

		meshLODStagingVertexBuffer.Map();
		meshLODStagingIndexBuffer.Map();

		for (uSize i = 0; i < count; i++)
		{
			VulkanMultiBufferEntry	LODVertexEntry;
			VulkanMultiBufferEntry	LODIndexEntry;
			TerrainMeshVertex*		pLODVertexData;
			uInt16*					pLODIndexData;

			uSize lodVertexResolution	= (closeResolution / (i + 1)) + 1;
			uSize lodVertexSizeBytes	= (lodVertexResolution * lodVertexResolution) * sizeof(TerrainMeshVertex);
			meshLODStagingVertexBuffer.Allocate(lodVertexSizeBytes / sizeof(TerrainMeshVertex), LODVertexEntry, &pLODVertexData);

			uSize lodIndexResolution	= (closeResolution / (i + 1));
			uSize lodIndexSizeBytes		= (lodIndexResolution * lodIndexResolution) * sizeof(uInt16) * 6;
			meshLODStagingIndexBuffer.Allocate(lodIndexSizeBytes / sizeof(uInt16), LODIndexEntry, &pLODIndexData);

			ModelData lodTerrainMesh = CreateTileMesh(lodIndexResolution);

			memcpy_s(pLODVertexData, lodVertexSizeBytes, lodTerrainMesh.vertices.Data(), lodVertexSizeBytes);
			memcpy_s(pLODIndexData, lodIndexSizeBytes, lodTerrainMesh.indices.Data(), lodIndexSizeBytes);

			TerrainLOD lod = {};
			lod.vertexEntry		= LODVertexEntry;
			lod.indexEntry		= LODIndexEntry;
			lod.pVertexBuffer	= mpLODVertexBuffer;
			lod.pIndexBuffer	= mpLODIndexBuffer;

			mLODs.PushBack(lod);
		}

		meshLODStagingVertexBuffer.Unmap();
		meshLODStagingIndexBuffer.Unmap();

		/* Transfer Buffer Data */

		mImmediateRecorder.Reset();
		mImmediateRecorder.BeginRecording();

		mImmediateRecorder.CopyBuffer(pMeshLODStagingVertexBuffer, mpLODVertexBuffer, meshLODVertexBufferInfo.sizeBytes, 0, 0);
		mImmediateRecorder.CopyBuffer(pMeshLODStagingIndexBuffer, mpLODIndexBuffer, meshLODIndexBufferInfo.sizeBytes, 0, 0);

		mImmediateRecorder.EndRecording();

		VulkanSubmission submission = {};
		submission.commandBuffers	= { mpTerrainCommandBuffer };
		submission.signalSemaphores = {};
		submission.waitSemaphores	= {};
		submission.waitStages		= {};

		mpGraphics->Submit(submission, device.queues.graphics, VK_NULL_HANDLE);

		/* Destroy Staging Buffers */

		resources.DestroyBuffer(pMeshLODStagingVertexBuffer);
		resources.DestroyBuffer(pMeshLODStagingIndexBuffer);
	}

	TerrainTile VulkanTerrainRenderer::CreateTile(uInt32 lodIndex, Vec2i position, float scale, uInt64 seed)
	{
		TerrainTileTextures textures = GenerateTileTextures(lodIndex, { (float)position.x, (float)position.y }, scale, seed);

		TerrainTile tile = {};
		tile.position	= position;
		tile.scale		= scale;
		tile.lodIndex	= lodIndex;
		tile.textures	= textures;

		return tile;
	}

	void VulkanTerrainRenderer::DestroyTile(const TerrainTile& tile)
	{
		// @TODO: use a pool system
		mpGraphics->pResourceManager->DestroyBuffer(tile.textures.pHeightMapBuffer);
		mpGraphics->pResourceManager->DestroyImage(tile.textures.pHeightMapImage);
		mpGraphics->pResourceManager->DestroyImageView(tile.textures.pHeightMapView);
	}

	void VulkanTerrainRenderer::UpdateGrid(const Vec2f& centerPos)
	{
		constexpr const sSize maxChunkDist = 3;

		sSize tileX = (sSize)centerPos.x;
		sSize tileY = (sSize)centerPos.y;

		Vec2i tileCenterPos(tileX, tileY);

		// Destroy tiles outside of range
		for (TerrainTile& tile : mActiveTiles)
		{
			Vec2i dist = tile.position - tileCenterPos;
			if (abs(dist.Magnitude()) > maxChunkDist)
			{
				DestroyTile(tile);
				mActiveTileMap.Remove(tile.position);
			}
		}

		mActiveTiles.Clear();

		for (sSize y = tileY - maxChunkDist; y <= tileY + maxChunkDist; y++)
		{
			for (sSize x = tileX - maxChunkDist; x <= tileX + maxChunkDist; x++)
			{
				Vec2i tilePosition(x, y);
				Vec2i dist = tilePosition - tileCenterPos;

				if (abs(dist.Magnitude()) <= (float)maxChunkDist)
				{
					TerrainTile tile;

					auto& tileIt = mActiveTileMap.Find(Vec2i{x, y});
					if (tileIt != mActiveTileMap.End())
					{
						tile = tileIt->value;
					}
					else
					{
						tile = CreateTile(0, { x, y }, 1.0f, 1234);
						mActiveTileMap.Put(tile.position, tile);
					}

					mActiveTiles.PushBack(tile);
				}
			}
		}
	}

	TerrainTileTextures VulkanTerrainRenderer::GenerateTileTextures(uInt32 lodIndex, const Vec2f& position, float scale, uInt64 seed)
	{
		VulkanResourceManager& resources = *mpGraphics->pResourceManager;
		VulkanDevice& device = *mpGraphics->pPrimaryDevice;

		/* Generate Heightmap Images */

		uSize perlinResolution = 25;//250;

		Array<float> perlin = GeneratePerlinNoise(
			perlinResolution, position.x * (float)perlinResolution, position.y * (float)perlinResolution, seed,
			{ 1.0f, -0.5f, 0.125f, 0.15f, 0.1f, 0.05f, 0.05f, 0.01f, 0.01f }
		);

		VulkanImageInfo perlinImageInfo = {};
		perlinImageInfo.vkImageType		= VK_IMAGE_TYPE_2D;
		perlinImageInfo.vkFormat		= VK_FORMAT_R32_SFLOAT;
		perlinImageInfo.vkUsageFlags	= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
		perlinImageInfo.width			= perlinResolution;
		perlinImageInfo.height			= perlinResolution;
		perlinImageInfo.depth			= 1;
		perlinImageInfo.layers			= 1;
		perlinImageInfo.mips			= 1;

		VulkanImage* pPerlinImage = resources.CreateImage(&device, perlinImageInfo);

		VulkanImageViewInfo perlinImageViewInfo = {};
		perlinImageViewInfo.pImage				= pPerlinImage;
		perlinImageViewInfo.vkImageViewType		= VK_IMAGE_VIEW_TYPE_2D;
		perlinImageViewInfo.vkAspectFlags		= VK_IMAGE_ASPECT_COLOR_BIT;
		perlinImageViewInfo.vkFormat			= VK_FORMAT_R32_SFLOAT;
		perlinImageViewInfo.mipStart			= 0;
		perlinImageViewInfo.mipCount			= 1;
		perlinImageViewInfo.layerStart			= 0;
		perlinImageViewInfo.layerCount			= 1;

		VulkanImageView* pPerlinImageView = resources.CreateImageView(&device, perlinImageViewInfo);

		VulkanBufferInfo perlinImageBufferInfo = {};
		perlinImageBufferInfo.sizeBytes				= perlinResolution * perlinResolution * sizeof(float);
		perlinImageBufferInfo.vkBufferUsage			= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
		perlinImageBufferInfo.vkMemoryProperties	= VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

		VulkanBuffer* pPerlinImageBuffer = resources.CreateBuffer(&device, perlinImageBufferInfo);

		VulkanBufferWriter perlinWriter(pPerlinImageBuffer);
		float* pPerlinData = perlinWriter.Map<float>();

		memcpy_s(pPerlinData, perlin.Size() * sizeof(float), perlin.Data(), perlin.Size() * sizeof(float));

		perlinWriter.Unmap();

		VkSamplerCreateInfo perlinSamplerInfo{};
		perlinSamplerInfo.sType				= VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		perlinSamplerInfo.magFilter			= VK_FILTER_LINEAR;
		perlinSamplerInfo.minFilter			= VK_FILTER_LINEAR;
		perlinSamplerInfo.addressModeU		= VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
		perlinSamplerInfo.addressModeV		= VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
		perlinSamplerInfo.addressModeW		= VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
		perlinSamplerInfo.anisotropyEnable	= VK_FALSE;
		perlinSamplerInfo.maxAnisotropy		= 1;
		perlinSamplerInfo.borderColor		= VK_BORDER_COLOR_INT_OPAQUE_BLACK;
		perlinSamplerInfo.unnormalizedCoordinates = VK_FALSE;
		perlinSamplerInfo.compareEnable		= VK_FALSE;
		perlinSamplerInfo.compareOp			= VK_COMPARE_OP_ALWAYS;
		perlinSamplerInfo.mipmapMode		= VK_SAMPLER_MIPMAP_MODE_LINEAR;
		perlinSamplerInfo.mipLodBias		= 0.0f;
		perlinSamplerInfo.minLod			= 0.0f;
		perlinSamplerInfo.maxLod			= 0.0f;

		// @TODO
		vkCreateSampler(device.vkDevice, &perlinSamplerInfo, VK_NULL_HANDLE, &mVkSampler);

		/* Generate Command Buffers */

		mImmediateRecorder.Reset();
		mImmediateRecorder.BeginRecording();

		VkImageMemoryBarrier vkPerlinMemoryBarrier = {};
		vkPerlinMemoryBarrier.sType								= VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		vkPerlinMemoryBarrier.srcAccessMask						= 0;
		vkPerlinMemoryBarrier.dstAccessMask						= VK_ACCESS_TRANSFER_WRITE_BIT;
		vkPerlinMemoryBarrier.oldLayout							= VK_IMAGE_LAYOUT_UNDEFINED;
		vkPerlinMemoryBarrier.newLayout							= VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		vkPerlinMemoryBarrier.image								= pPerlinImage->vkImage;
		vkPerlinMemoryBarrier.subresourceRange.aspectMask		= VK_IMAGE_ASPECT_COLOR_BIT;
		vkPerlinMemoryBarrier.subresourceRange.baseMipLevel		= 0;
		vkPerlinMemoryBarrier.subresourceRange.levelCount		= 1;
		vkPerlinMemoryBarrier.subresourceRange.baseArrayLayer	= 0;
		vkPerlinMemoryBarrier.subresourceRange.layerCount		= 1;

		VulkanPipelineBarrierInfo perlinBarrierInfo = {};
		perlinBarrierInfo.srcStage					= VK_PIPELINE_STAGE_TRANSFER_BIT;
		perlinBarrierInfo.dstStage					= VK_PIPELINE_STAGE_TRANSFER_BIT;
		perlinBarrierInfo.dependencyFlags			= 0;
		perlinBarrierInfo.memoryBarrierCount		= 0;
		perlinBarrierInfo.pMemoryBarriers			= nullptr;
		perlinBarrierInfo.bufferMemoryBarrierCount	= 0;
		perlinBarrierInfo.pBufferMemoryBarriers		= nullptr;
		perlinBarrierInfo.imageMemoryBarrierCount	= 1;
		perlinBarrierInfo.pImageMemoryBarriers		= &vkPerlinMemoryBarrier;

		mImmediateRecorder.PipelineBarrier(perlinBarrierInfo);

		VkBufferImageCopy vkPerlinImageCopy = {};
		vkPerlinImageCopy.bufferOffset						= 0;
		vkPerlinImageCopy.bufferRowLength					= 0;
		vkPerlinImageCopy.bufferImageHeight					= 0;
		vkPerlinImageCopy.imageSubresource.aspectMask		= VK_IMAGE_ASPECT_COLOR_BIT;
		vkPerlinImageCopy.imageSubresource.baseArrayLayer	= 0;
		vkPerlinImageCopy.imageSubresource.layerCount		= 1;
		vkPerlinImageCopy.imageSubresource.mipLevel			= 0;
		vkPerlinImageCopy.imageOffset.x						= 0;
		vkPerlinImageCopy.imageOffset.y						= 0;
		vkPerlinImageCopy.imageOffset.z						= 0;
		vkPerlinImageCopy.imageExtent.width					= perlinResolution;
		vkPerlinImageCopy.imageExtent.height				= perlinResolution;
		vkPerlinImageCopy.imageExtent.depth					= 1;

		mImmediateRecorder.CopyBufferToImage(pPerlinImageBuffer, pPerlinImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, { vkPerlinImageCopy });

		VkImageMemoryBarrier vkPerlinMemoryBarrier2 = {};
		vkPerlinMemoryBarrier2.sType							= VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		vkPerlinMemoryBarrier2.srcAccessMask					= 0;
		vkPerlinMemoryBarrier2.dstAccessMask					= VK_ACCESS_SHADER_READ_BIT;
		vkPerlinMemoryBarrier2.oldLayout						= VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		vkPerlinMemoryBarrier2.newLayout						= VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		vkPerlinMemoryBarrier2.image							= pPerlinImage->vkImage;
		vkPerlinMemoryBarrier2.subresourceRange.aspectMask		= VK_IMAGE_ASPECT_COLOR_BIT;
		vkPerlinMemoryBarrier2.subresourceRange.baseMipLevel	= 0;
		vkPerlinMemoryBarrier2.subresourceRange.levelCount		= 1;
		vkPerlinMemoryBarrier2.subresourceRange.baseArrayLayer	= 0;
		vkPerlinMemoryBarrier2.subresourceRange.layerCount		= 1;

		VulkanPipelineBarrierInfo perlinBarrierInfo2 = {};
		perlinBarrierInfo2.srcStage					= VK_PIPELINE_STAGE_VERTEX_SHADER_BIT;
		perlinBarrierInfo2.dstStage					= VK_PIPELINE_STAGE_VERTEX_SHADER_BIT;
		perlinBarrierInfo2.dependencyFlags			= 0;
		perlinBarrierInfo2.memoryBarrierCount		= 0;
		perlinBarrierInfo2.pMemoryBarriers			= nullptr;
		perlinBarrierInfo2.bufferMemoryBarrierCount	= 0;
		perlinBarrierInfo2.pBufferMemoryBarriers	= nullptr;
		perlinBarrierInfo2.imageMemoryBarrierCount	= 1;
		perlinBarrierInfo2.pImageMemoryBarriers		= &vkPerlinMemoryBarrier2;

		mImmediateRecorder.PipelineBarrier(perlinBarrierInfo2);

		mImmediateRecorder.EndRecording();

		VulkanSubmission renderSubmition	= {};
		renderSubmition.commandBuffers		= { mpTerrainCommandBuffer };
		renderSubmition.waitSemaphores		= {};
		renderSubmition.waitStages			= {};
		renderSubmition.signalSemaphores	= {};

		mpGraphics->Submit(renderSubmition, device.queues.graphics, VK_NULL_HANDLE);

		TerrainTileTextures textures = {};
		textures.pHeightMapBuffer	= pPerlinImageBuffer;
		textures.pHeightMapImage	= pPerlinImage;
		textures.pHeightMapView		= pPerlinImageView;

		return textures;
	}

	VulkanTerrainRenderer::VulkanTerrainRenderer() :
		mImmediateRecorder(nullptr) { }

	void VulkanTerrainRenderer::Initialize(VulkanGraphics& graphics, VulkanShaderCache& shaderCache, VulkanPipelineCache& pipelineCache)
	{
		mpGraphics = &graphics;

		/* Create Command Buffers */

		VulkanCommandPoolInfo terrainCommandPoolInfo = {};
		terrainCommandPoolInfo.queueFamilyIndex			= graphics.pPrimaryDevice->pPhysicalDevice->primaryQueueFamilyIndices.graphics;
		terrainCommandPoolInfo.vkCommandPoolCreateFlags	= VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

		mpTerrainCommandPool = graphics.pResourceManager->CreateCommandPool(graphics.pPrimaryDevice, terrainCommandPoolInfo);
		graphics.pResourceManager->CreateCommandBuffers(mpTerrainCommandPool, 1, &mpTerrainCommandBuffer);
		mImmediateRecorder = VulkanCommandRecorder(mpTerrainCommandBuffer);

		/* Create LOD Data */

		CreateLODs(4, 200);

		/* Create Pipelines */

		VulkanShader* pTerrainVertexShader = shaderCache.FindOrCreateShader("Shaders/terrain.vert", VK_SHADER_STAGE_VERTEX_BIT);
		VulkanShader* pTerrainFragmentShader = shaderCache.FindOrCreateShader("Shaders/terrain.frag", VK_SHADER_STAGE_FRAGMENT_BIT);

		Array<VulkanAttachment> attachments =
		{
			{ "Swapchain",		VULKAN_ATTACHMENT_TYPE_SWAPCHAIN,		VK_FORMAT_B8G8R8A8_UNORM },
			{ "Depth-Stencil",	VULKAN_ATTACHMENT_TYPE_DEPTH_STENCIL,	VK_FORMAT_D24_UNORM_S8_UINT }
		};

		VkVertexInputBindingDescription vertexBufferAttachment = {};
		vertexBufferAttachment.binding		= 0;
		vertexBufferAttachment.stride		= sizeof(TerrainMeshVertex);
		vertexBufferAttachment.inputRate	= VK_VERTEX_INPUT_RATE_VERTEX;

		Array<VkVertexInputBindingDescription> vertexBindings;

		vertexBindings.PushBack(vertexBufferAttachment);

		VkVertexInputAttributeDescription positionAttrib = {};
		positionAttrib.binding				= 0;
		positionAttrib.location				= 0;
		positionAttrib.format				= VK_FORMAT_R32G32B32_SFLOAT;
		positionAttrib.offset				= 0;

		Array<VkVertexInputAttributeDescription> vertexAttributes;
		vertexAttributes.PushBack(positionAttrib);

		mpTerrainRenderPipeline = pipelineCache.FindOrCreateGraphicsPipeline(
			{ pTerrainVertexShader, pTerrainFragmentShader },
			attachments, vertexAttributes, vertexBindings
		);
	}

	void VulkanTerrainRenderer::Update(const Vec2f& gridPos, CameraComponent& camera, TransformComponent& cameraTransform)
	{
		constexpr const float globalTerrainScale = 5.0f;

		UpdateGrid(gridPos / globalTerrainScale);

		for (uSize i = 0; i < mActiveTiles.Size(); i++)
		{
			float tileScale		= mActiveTiles[i].scale;
			Vec2i tilePos		= mActiveTiles[i].position * tileScale;
			Vec3f worldPos		= Vec3f{ (float)tilePos.x, 0.0f, (float)tilePos.y } *globalTerrainScale;
			Vec3f worldScale	= Vec3f{ tileScale, tileScale, tileScale } * globalTerrainScale;

			TransformComponent tileTransform(worldPos, { {0.0f, 0.0f, 0.0f}, 0.0f }, worldScale);

			mpPerTileDatas[i].model	= tileTransform.GetMatrix();
			mpPerTileDatas[i].view	= cameraTransform.GetViewMatrix();
			mpPerTileDatas[i].proj	= camera.GetProjectionMatrix();
		}
	}

	void VulkanTerrainRenderer::RecordTransfers(VulkanCommandRecorder& transferRecorder)
	{
		transferRecorder.CopyBuffer(mpPerTileStagingBuffer, mpPerTileBuffer, mpPerTileBuffer->sizeBytes, 0, 0);
	}

	void VulkanTerrainRenderer::RecordDraws(VulkanCommandRecorder& renderRecorder)
	{
		renderRecorder.SetGraphicsPipeline(mpTerrainRenderPipeline);

		VulkanBufferBind terrainVertexBind = {};
		terrainVertexBind.pBuffer	= mpLODVertexBuffer;
		terrainVertexBind.offset	= 0;

		VulkanBufferBind terrainIndexBind = {};	// @TODO: change SetIndexBuffer to use this
		terrainIndexBind.pBuffer	= mpLODIndexBuffer;
		terrainIndexBind.offset		= 0;

		renderRecorder.SetVertexBuffers(&terrainVertexBind, 1);
		renderRecorder.SetIndexBuffer(mpLODIndexBuffer, 0, VK_INDEX_TYPE_UINT16);

		for (uSize i = 0; i < mActiveTiles.Size(); i++)
		{
			uInt32 lodIndex = mActiveTiles[i].lodIndex;
			TerrainLOD& lod = mLODs[lodIndex];

			VulkanUniformBufferBind binding = {};
			binding.binding = 0;
			binding.pBuffer = mpPerTileBuffer;
			binding.offset	= i * sizeof(TerrainPerTileData);
			binding.range	= sizeof(TerrainPerTileData);

			VulkanUniformImageBind imageBinding = {};
			imageBinding.binding	= 1;
			imageBinding.vkSampler	= mVkSampler;
			imageBinding.pImageView = mActiveTiles[i].textures.pHeightMapView;
			imageBinding.vkLayout	= VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

			VulkanUniformBufferBind pBufferBinds[] = { binding };
			VulkanUniformImageBind pImageBinds[] = { imageBinding };

			renderRecorder.BindUniforms(mpTerrainRenderPipeline, 0, pBufferBinds, 1, pImageBinds, 1);

			uSize indexCount = lod.indexEntry.sizeBytes / sizeof(uInt16);
			uSize indexStart = lod.indexEntry.offset / sizeof(uInt16);
			renderRecorder.DrawIndexed(1, indexCount, indexStart, lod.vertexEntry.offset / sizeof(TerrainMeshVertex));
		}
	}

	Array<float> VulkanTerrainRenderer::GeneratePerlinNoise(uSize resolution, float offsetX, float offsetY, uInt64 seed, const Array<float>& octaveWeights)
	{
		Array<float> finalNoise(resolution * resolution);

		for (float y = 0; y < resolution; y++)
		{
			for (float x = 0; x < resolution; x++)
			{
				float freq		= 1.0f;
				float maxAmp	= 0.0f;
				float value		= 0.0f;
				float shiftX	= 112.233f;
				float shiftY	= 121.323f;

				for (float amplitude : octaveWeights)
				{
					constexpr float gridSize = 250.0f;

					float seedX = shiftX + (offsetX + x) * (freq / gridSize);
					float seedY = shiftY + (offsetY + y) * (freq / gridSize);

					value += PerlinNoise2D(seedX, seedY) * amplitude;

					freq *= 1.5f;
					maxAmp += amplitude;
					shiftX += 112.233f;
					shiftY += 121.323f;
				}

				finalNoise[x + y * resolution] = (0.5f + value / 2.0f) / 2.0f;
			}
		}

		return finalNoise;
	}
}

