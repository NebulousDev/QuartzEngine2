#pragma once

#include "VulkanResourceManager.h"
#include "VulkanCommandRecorder.h"
#include "VulkanShaderCache.h"
#include "VulkanPipelineCache.h"
#include "VulkanMultiBuffer.h"
#include "Entity/World.h"
#include "Types/Map.h"

#include "Component/MeshComponent.h"
#include "Component/TransformComponent.h"
#include "VulkanRenderable.h"
#include "VulkanMaterial.h"

namespace Quartz
{
	struct VulkanRenderSettings
	{
		bool useUniqueMeshBuffers;
		bool useUniqueMeshStagingBuffers;
		bool useUniqueUniformBuffers;
		bool useUniqueUniformStagingBuffers;
		bool usePerModelPushConstants; //TODO

		struct
		{
			uSize vertexBufferSizeMb;
			uSize indexBufferSizeMb;
			uSize perInstanceBufferSizeMb;
			uSize perModelBufferSizeMb;

			uSize globalBufferSizeBytes;
			uSize uniquePerInstanceBufferSizeBytes; //
			uSize uniquePerModelBufferSizeBytes; //
		};

		bool useInstancing; // TODO
		bool useMeshStaging;
		bool useUniformStaging;
		bool useDrawIndirect; // TODO
	};

	class VulkanBufferCache
	{
	private:
		struct TransferCommand
		{
			VulkanMultiBuffer*		pSrcBuffer;
			VulkanMultiBuffer*		pDestBuffer;
			VulkanMultiBufferEntry	srcEntry;
			VulkanMultiBufferEntry	destEntry;
		};

	private:
		VulkanResourceManager*		mpResourceManager;
		VulkanDevice*				mpDevice;

		Array<VulkanMultiBuffer>	mVertexBuffers;
		Array<VulkanMultiBuffer>	mVertexStagingBuffers;
		Array<VulkanMultiBuffer>	mIndexBuffers;
		Array<VulkanMultiBuffer>	mIndexStagingBuffers;

		VulkanBufferWriter			mGlobalBuffer;
		VulkanBufferWriter			mGlobalStagingBuffer;
		Array<VulkanMultiBuffer>	mPerInstanceBuffers;
		Array<VulkanMultiBuffer>	mPerInstanceStagingBuffers;
		Array<VulkanMultiBuffer>	mPerModelBuffers;
		Array<VulkanMultiBuffer>	mPerModelStagingBuffers;

		Map<uInt64, MeshBufferLocation>	mMeshBufferLookup;
		Map<uInt64, MeshBufferLocation>	mMeshStagingBufferLookup;
		Array<VulkanRenderable>			mRenderables;
		Array<VulkanRenderable>			mRenderablesSorted;
		Array<TransferCommand>			mReadyTransfers;

		VulkanRenderSettings		mSettings;

	private:
		void					InitializeDefaultBuffers();

		MeshBufferLocation		GetOrAllocateMeshBuffers(uInt64 meshHash, const ModelData* pModelData, bool& outFound);
		MeshBufferLocation		GetOrAllocateMeshStagingBuffers(uInt64 meshHash, const ModelData* pModelData, bool& outFound);
		PerModelBufferLocation	AllocatePerModelBuffer(void* pPerModelData, uSize perModelSizeBytes);
		PerModelBufferLocation	AllocatePerModelStagingBuffer(void* pPerModelData, uSize perModelSizeBytes);

	public:
		void					Initialize(VulkanDevice* pDevice, VulkanResourceManager* pResourceManager, const VulkanRenderSettings& settings);

		void					ResetPerModelBuffers();

		void					FillRenderableVertexData(VulkanRenderable& renderable, uInt64 meshHash, const ModelData* pModelData, bool& outFound);
		void					FillRenderablePerModelData(VulkanRenderable& renderable, uInt64 renderableId, void* pPerModelData, uSize perModelSizeBytes);

		void					RecordTransfers(VulkanCommandRecorder* pRecorder);
	};
}