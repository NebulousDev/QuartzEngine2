#pragma once

#include "Types/Array.h"
#include "Types/Stack.h"
#include "GfxAPI.h"
#include "ModelData.h"

#include "Vulkan/VulkanGraphics.h"
#include "Vulkan/VulkanMultiBuffer.h"
#include "Vulkan/VulkanResourceManager.h"
#include "Vulkan/VulkanCommandRecorder.h"
#include "Vulkan/VulkanPipelineCache.h"
#include "Vulkan/VulkanShaderCache.h"

#include "Component/CameraComponent.h"
#include "Component/TransformComponent.h"

namespace Quartz
{
	struct TerrainMeshVertex
	{
		Vec3f position;
	};

	struct TerrainPerTileData
	{
		Mat4f model;
		Mat4f view;
		Mat4f proj;
	};

	struct TerrainLOD
	{
		VulkanMultiBufferEntry	vertexEntry;
		VulkanMultiBufferEntry	indexEntry;
		VulkanBuffer*			pVertexBuffer;
		VulkanBuffer*			pIndexBuffer;
	};

	struct TerrainTileTextures
	{
		VulkanBuffer*		pHeightMapBuffer;
		VulkanImage*		pHeightMapImage;
		VulkanImageView*	pHeightMapView;
	};

	struct TerrainTile
	{
		Vec2i				position;
		float				scale;
		uInt32				lodIndex;
		TerrainTileTextures	textures;
		bool				ready;
	};

	class QUARTZ_GRAPHICS_API VulkanTerrainRenderer
	{
	private:
		VulkanGraphics*				mpGraphics;
		VulkanBuffer*				mpLODVertexBuffer;
		VulkanBuffer*				mpLODIndexBuffer;
		Array<TerrainLOD>			mLODs;
		Array<TerrainTile>			mActiveTiles;
		Map<Vec2i, TerrainTile>		mActiveTileMap;
		Stack<TerrainTile>			mLoadingTiles;

		VulkanBuffer*				mpPerTileStagingBuffer; //TODO: Should be buffered?
		VulkanBuffer*				mpPerTileBuffer;
		VulkanBufferWriter			mpPerTileWriter;
		TerrainPerTileData*			mpPerTileDatas;

		VulkanCommandPool*			mpImmediateCommandPool;
		VulkanCommandBuffer*		mImmediateCommandBuffers[3];
		VulkanCommandRecorder		mImmediateRecorders[3];
		VkFence						mImmediateFences[3];
		uSize						mImmediateIdx;

		VulkanGraphicsPipeline*		mpTerrainRenderPipeline;

		VkSampler					mVkSampler;

	private:
		ModelData	CreateTileMesh(uSize resolution);
		void		CreateLODs(uSize count, uSize closeResolution);

		TerrainTile	CreateTile(uInt32 lodIndex, Vec2i position, float scale, uInt64 seed);
		void		DestroyTile(const TerrainTile& tile);

		void		UpdateGrid(const Vec2f& centerPos);

		Array<float>		GeneratePerlinNoise(uSize resolution, float offsetX, float offsetY, uInt64 seed,
								float scale, float lacunarity, const Array<float>& octaveWeights);
		Array<float>		GeneratePerlinNoiseMT(uSize resolution, float offsetX, float offsetY, uInt64 seed,
								float scale, float lacunarity, const Array<float>& octaveWeights);
		TerrainTileTextures	GenerateTileTextures(uInt32 lodIndex, const Vec2f& position, float scale, uInt64 seed, uSize resolution);

	public:
		void Initialize(VulkanGraphics& graphics, VulkanShaderCache& shaderCache, VulkanPipelineCache& pipelineCache);

		void Update(const Vec2f& gridPos, CameraComponent& camera, TransformComponent& cameraTransform);

		void RecordTransfers(VulkanCommandRecorder& transferRecorder);
		void RecordDraws(VulkanCommandRecorder& renderRecorder);
	};
}