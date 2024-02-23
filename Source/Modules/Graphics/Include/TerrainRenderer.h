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

namespace Quartz
{
	struct TerrainMeshVertex
	{
		Vec3f position;
	};

	struct TerrainPerChunkData
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

	class QUARTZ_GRAPHICS_API VulkanTerrainRenderer
	{
	private:
		VulkanBuffer*			mpLODVertexBuffer;
		VulkanBuffer*			mpLODIndexBuffer;
		Array<TerrainLOD>		mLODs;

		VulkanBuffer*			mpPerChunkStagingBuffer;
		VulkanBuffer*			mpPerChunkBuffer;
		VulkanBufferWriter		mpPerChunkWriter;
		TerrainPerChunkData*	mpPerChunkDatas;

		VulkanCommandPool*		mpTerrainCommandPool;
		VulkanCommandBuffer*	mpTerrainCommandBuffer;
		VulkanCommandRecorder	mImmediateRecorder;

		VulkanGraphicsPipeline* mpTerrainRenderPipeline;

		VulkanImageView*		mpPerlinImageView;
		VkSampler				mVkSampler;

	private:
		ModelData CreateChunkMesh(uSize resolution);
		void CreateLODs(uSize count, uSize closeResolution, VulkanGraphics& graphics);

		Array<float> CreatePerlinNoise(uSize resolution, uInt64 seed, const Array<float>& octaveWeights);
		void CreateLodTextures(VulkanGraphics& graphics);

	public:
		VulkanTerrainRenderer();

		void Initialize(VulkanGraphics& graphics, VulkanShaderCache& shaderCache, VulkanPipelineCache& pipelineCache);

		void Update(CameraComponent& camera, TransformComponent& cameraTransform);

		void RecordTransfers(VulkanCommandRecorder& transferRecorder);
		void RecordDraws(VulkanCommandRecorder& renderRecorder);
	};
}