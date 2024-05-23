#pragma once

#include "Types/Map.h"
#include "Entity/World.h"

#include "VulkanResourceManager.h"
#include "VulkanCommandRecorder.h"
#include "VulkanShaderCache.h"
#include "VulkanPipelineCache.h"
#include "VulkanMultiBuffer.h"
#include "VulkanRenderable.h"
#include "VulkanMaterial.h"

#include "Component/MeshComponent.h"
#include "Component/TransformComponent.h"


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
			uSize uniformBufferSizeMb;

			uSize globalBufferSizeBytes;
			uSize uniquePerInstanceBufferSizeBytes; //
			uSize uniquePerModelBufferSizeBytes; //
		};

		uSize maxUniformSets;

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
		Array<VulkanMultiBuffer>	mUniformBuffers;
		Array<VulkanMultiBuffer>	mUniformStagingBuffers;

		Map<AssetID, MeshBufferLocation>	mMeshBufferLookup;
		Map<AssetID, MeshBufferLocation>	mMeshStagingBufferLookup;
		Array<VulkanRenderable>				mRenderables;
		Array<VulkanRenderable>				mRenderablesSorted;
		Array<TransferCommand>				mReadyTransfers;

		VulkanRenderSettings		mSettings;

	private:
		void InitializeDefaultBuffers();
			 
		bool GetOrAllocateMeshBuffers(const Model& model, MeshBufferLocation& outBufferLocation, 
			 	uSize vertexAlignBytes, uSize indexAlignBytes, bool& outFound);
		bool GetOrAllocateMeshStagingBuffers(const Model& model, MeshBufferLocation& outStagingBufferLocation, 
			 	uSize vertexAlignBytes, uSize indexAlignBytes, bool& outFound);
		bool AllocateUniformBuffer(UniformBufferLocation& outUniformBuffer, uSize set, void* pPerModelData, uSize perModelSizeBytes);
		bool AllocateUniformStagingBuffer(UniformBufferLocation& outUniformBuffer, uSize set, void* pPerModelData, uSize perModelSizeBytes);

	public:
		void Initialize(VulkanDevice* pDevice, VulkanResourceManager* pResourceManager, const VulkanRenderSettings& settings);
			 
		void ResetPerModelBuffers();
			 
		bool GetOrAllocateBuffers(const Model& model, MeshBufferLocation& outbufferLocation,
			 	MeshBufferLocation& outStagingbufferLocation, uSize vertexAlignBytes, uSize indexAlignBytes, bool& outFound);
		bool AllocateAndWriteUniformData(UniformBufferLocation& outUniformBuffer, uSize set, void* pUniformData, uSize uniformSizeBytes);
			 
		void RecordTransfers(VulkanCommandRecorder& recorder);
	};
}