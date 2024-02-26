#pragma once

#include "Types/Array.h"
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

//#include "Types/FractalGrid.h"

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
		Vec2f				position;
		float				scale;
		uInt32				lodIndex;
		TerrainTileTextures	textures;
	};

	class QUARTZ_GRAPHICS_API VulkanTerrainRenderer
	{
	private:
		VulkanBuffer*				mpLODVertexBuffer;
		VulkanBuffer*				mpLODIndexBuffer;
		Array<TerrainLOD>			mLODs;
		//FractalGrid<TerrainTile>	mTileGrid;
		Array<TerrainTile>			mActiveTiles;

		VulkanBuffer*				mpPerTileStagingBuffer;
		VulkanBuffer*				mpPerTileBuffer;
		VulkanBufferWriter			mpPerTileWriter;
		TerrainPerTileData*			mpPerTileDatas;

		VulkanCommandPool*			mpTerrainCommandPool;
		VulkanCommandBuffer*		mpTerrainCommandBuffer;
		VulkanCommandRecorder		mImmediateRecorder;

		VulkanGraphicsPipeline*		mpTerrainRenderPipeline;

		VkSampler					mVkSampler;

	private:
		ModelData	CreateTileMesh(uSize resolution);
		void		CreateLODs(uSize count, uSize closeResolution, VulkanGraphics& graphics);

		TerrainTile	CreateTile(VulkanGraphics& graphics, uInt32 lodIndex, Vec2f position, float scale, uInt64 seed);
		void		DestroyTile(VulkanGraphics& graphics, const TerrainTile& tile);

		Array<float>		GeneratePerlinNoise(uSize resolution, float offsetX, float offsetY, uInt64 seed, const Array<float>& octaveWeights);
		TerrainTileTextures	GenerateTileTextures(VulkanGraphics& graphics, uInt32 lodIndex, const Vec2f& position, float scale, uInt64 seed);

	public:
		VulkanTerrainRenderer();

		void Initialize(VulkanGraphics& graphics, VulkanShaderCache& shaderCache, VulkanPipelineCache& pipelineCache);

		void Update(const Vec3f& gridPos, CameraComponent& camera, TransformComponent& cameraTransform);

		void RecordTransfers(VulkanCommandRecorder& transferRecorder);
		void RecordDraws(VulkanCommandRecorder& renderRecorder);
	};
}