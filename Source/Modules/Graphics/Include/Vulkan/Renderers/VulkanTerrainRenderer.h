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

	enum TerrainState
	{
		TERRAIN_STATE_WAITING,
		TERRAIN_STATE_LOADING,
		TERRAIN_STATE_ACTIVE,
		TERRAIN_STATE_UNLOADING,
		TERRAIN_STATE_UNLOADED
	};

	struct TerrainTile
	{
		Vec2i				position;
		float				scale;
		uInt32				lodIndex;
		TerrainTileTextures	textures;
		TerrainState		state;
		bool				ready;
	};

	class QUARTZ_GRAPHICS_API VulkanTerrainRenderer
	{
	private:
		VulkanGraphics*				mpGraphics;
		VulkanDevice*				mpDevice;

		VulkanBuffer*				mpLODVertexBuffer;
		VulkanBuffer*				mpLODIndexBuffer;
		Array<TerrainLOD>			mLODs;
		Array<TerrainTile>			mActiveTiles;
		Stack<TerrainTile>			mUnloadingTiles;
		Map<Vec2i, TerrainTile>		mActiveTileMap;
		Stack<TerrainTile>			mLoadingTiles;

		VulkanBuffer*				mpPerTileStagingBuffer; //TODO: Should be buffered?
		VulkanBuffer*				mpPerTileBuffer;
		VulkanBufferWriter			mpPerTileWriter;
		TerrainPerTileData*			mpPerTileDatas;

		VulkanCommandPool*			mpImmediateCommandPool;
		VulkanCommandBuffer*		mImmediateCommandBuffers[VULKAN_GRAPHICS_MAX_IN_FLIGHT]; // @TODO may have it's own max
		VulkanCommandRecorder		mImmediateRecorders[VULKAN_GRAPHICS_MAX_IN_FLIGHT];
		VkFence						mImmediateFences[VULKAN_GRAPHICS_MAX_IN_FLIGHT];
		uSize						mImmediateIdx;

		VulkanGraphicsPipeline*		mpTerrainRenderPipeline;

		VkSampler					mVkSampler;

	private:
		ModelData	CreateTileMesh(uSize resolution);
		void		CreateLODs(uSize count, uSize closeResolution);

		bool		CreateTile(TerrainTile& tile, Vec2i position, uInt32 lodIndex, uSize resolution, float scale, uInt64 seed);
		void		DestroyTile(const TerrainTile& tile);

		void		UpdateGrid(const Vec2f& centerPos);

		Array<float>		GeneratePerlinNoiseMT(uSize resolution, float offsetX, float offsetY, uInt64 seed,
								float scale, float lacunarity, const Array<float>& octaveWeights);
		TerrainTileTextures	GenerateTileTextures(uInt32 lodIndex, const Vec2f& position, 
								float scale, uInt64 seed, uSize resolution);

	public:
		void Initialize(VulkanGraphics& graphics, VulkanDevice& device, VulkanShaderCache& shaderCache,
			VulkanPipelineCache& pipelineCache, uSize maxInFlightCount);

		void Update(const Vec2f& gridPos, CameraComponent& camera, TransformComponent& cameraTransform);

		void RecordTransfers(VulkanCommandRecorder& transferRecorder);
		void RecordDraws(VulkanCommandRecorder& renderRecorder);
	};
}