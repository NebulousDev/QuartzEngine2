#include "Vulkan/VulkanBufferCache.h"

#include "Log.h"

namespace Quartz
{
	void VulkanBufferCache::InitializeDefaultBuffers()
	{
		VkBufferUsageFlags		usageFlags = 0;
		VkMemoryPropertyFlags	memoryFlags = 0;

		if (mSettings.useMeshStaging)
		{
			usageFlags	= VK_BUFFER_USAGE_TRANSFER_DST_BIT;
			memoryFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
		}
		else
		{
			usageFlags	= 0;
			memoryFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
		}

		if (!mSettings.useUniqueMeshBuffers)
		{
			VulkanBufferInfo vertexBufferInfo = {};
			vertexBufferInfo.sizeBytes			= mSettings.vertexBufferSizeMb * (1024 * 1024);
			vertexBufferInfo.vkBufferUsage		= VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | usageFlags;
			vertexBufferInfo.vkMemoryProperties = memoryFlags;

			mVertexBuffers.PushBack(VulkanMultiBuffer(mpResourceManager->CreateBuffer(mpDevice, vertexBufferInfo)));

			VulkanBufferInfo indexBufferInfo = {};
			indexBufferInfo.sizeBytes			= mSettings.indexBufferSizeMb * (1024 * 1024);
			indexBufferInfo.vkBufferUsage		= VK_BUFFER_USAGE_INDEX_BUFFER_BIT | usageFlags;
			indexBufferInfo.vkMemoryProperties	= memoryFlags;

			mIndexBuffers.PushBack(VulkanMultiBuffer(mpResourceManager->CreateBuffer(mpDevice, indexBufferInfo)));
		}

		VulkanBufferInfo globalBufferInfo = {};
		globalBufferInfo.sizeBytes				= mSettings.globalBufferSizeBytes;
		globalBufferInfo.vkBufferUsage			= VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | usageFlags;
		globalBufferInfo.vkMemoryProperties		= memoryFlags;

		mGlobalBuffer = mpResourceManager->CreateBuffer(mpDevice, globalBufferInfo);

		if (!mSettings.useUniqueUniformBuffers)
		{
			VulkanBufferInfo perInstanceBufferInfo = {};
			perInstanceBufferInfo.sizeBytes				= mSettings.perInstanceBufferSizeMb * (1024 * 1024);
			perInstanceBufferInfo.vkBufferUsage			= VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | usageFlags;
			perInstanceBufferInfo.vkMemoryProperties	= memoryFlags;

			mPerInstanceBuffers.PushBack(VulkanMultiBuffer(mpResourceManager->CreateBuffer(mpDevice, perInstanceBufferInfo)));

			VulkanBufferInfo perModelBufferInfo = {};
			perModelBufferInfo.sizeBytes			= mSettings.perModelBufferSizeMb * (1024 * 1024);
			perModelBufferInfo.vkBufferUsage		= VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | usageFlags;
			perModelBufferInfo.vkMemoryProperties	= memoryFlags;

			mPerModelBuffers.PushBack(VulkanMultiBuffer(mpResourceManager->CreateBuffer(mpDevice, perModelBufferInfo)));
		}

		if (!mSettings.useUniqueMeshStagingBuffers)
		{
			if (mSettings.useMeshStaging)
			{
				VulkanBufferInfo stagingVertexBufferInfo = {};
				stagingVertexBufferInfo.sizeBytes			= mSettings.vertexBufferSizeMb * (1024 * 1024);
				stagingVertexBufferInfo.vkBufferUsage		= VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
				stagingVertexBufferInfo.vkMemoryProperties	= VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

				mVertexStagingBuffers.PushBack(VulkanMultiBuffer(mpResourceManager->CreateBuffer(mpDevice, stagingVertexBufferInfo)));

				VulkanBufferInfo stagingGlobalBufferInfo = {};
				stagingGlobalBufferInfo.sizeBytes			= mSettings.perInstanceBufferSizeMb * (1024 * 1024);
				stagingGlobalBufferInfo.vkBufferUsage		= VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
				stagingGlobalBufferInfo.vkMemoryProperties	= VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

				mGlobalStagingBuffer = mpResourceManager->CreateBuffer(mpDevice, stagingGlobalBufferInfo);

				VulkanBufferInfo stagingIndexBufferInfo = {};
				stagingIndexBufferInfo.sizeBytes			= mSettings.indexBufferSizeMb * (1024 * 1024);
				stagingIndexBufferInfo.vkBufferUsage		= VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
				stagingIndexBufferInfo.vkMemoryProperties	= VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

				mIndexStagingBuffers.PushBack(VulkanMultiBuffer(mpResourceManager->CreateBuffer(mpDevice, stagingIndexBufferInfo)));

				VulkanBufferInfo stagingPerInstanceBufferInfo = {};
				stagingPerInstanceBufferInfo.sizeBytes			= mSettings.perInstanceBufferSizeMb * (1024 * 1024);
				stagingPerInstanceBufferInfo.vkBufferUsage		= VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
				stagingPerInstanceBufferInfo.vkMemoryProperties	= VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

				mPerInstanceStagingBuffers.PushBack(VulkanMultiBuffer(mpResourceManager->CreateBuffer(mpDevice, stagingPerInstanceBufferInfo)));

				mVertexStagingBuffers[0].Map();
				mIndexStagingBuffers[0].Map();
				mGlobalStagingBuffer.Map();
				mPerInstanceStagingBuffers[0].Map();
			}

			if (mSettings.useUniformStaging && !mSettings.usePerModelPushConstants)
			{
				VulkanBufferInfo stagingPerModelBufferInfo = {};
				stagingPerModelBufferInfo.sizeBytes				= mSettings.perModelBufferSizeMb * (1024 * 1024);
				stagingPerModelBufferInfo.vkBufferUsage			= VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
				stagingPerModelBufferInfo.vkMemoryProperties	= VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

				mPerModelStagingBuffers.PushBack(VulkanMultiBuffer(mpResourceManager->CreateBuffer(mpDevice, stagingPerModelBufferInfo)));

				mPerModelStagingBuffers[0].Map();
			}
		}
	}

	void VulkanBufferCache::Initialize(VulkanDevice* pDevice, VulkanResourceManager* pResourceManager, const VulkanRenderSettings& settings)
	{
		mSettings			= settings;
		mpResourceManager	= pResourceManager;
		mpDevice			= pDevice;

		InitializeDefaultBuffers();
	}

	void CopyMeshData(const VertexData& vertexData, void* pOutVertexData, void* pOutIndexData)
	{
		const uSize vertexSizeBytes	= vertexData.pVertexBuffer->Size();
		const uSize indexSizeBytes	= vertexData.pIndexBuffer->Size();
		const void* pVertexData		= vertexData.pVertexBuffer->Data();
		const void* pIndexData		= vertexData.pIndexBuffer->Data();

		memcpy_s(pOutVertexData, vertexSizeBytes, pVertexData, vertexSizeBytes);
		memcpy_s(pOutIndexData, indexSizeBytes, pIndexData, indexSizeBytes);
	}

	void CopyPerModelData(void* pPerModelData, void* pOutPerModelBuffer, uSize sizeBytes)
	{
		memcpy_s(pOutPerModelBuffer, sizeBytes, pPerModelData, sizeBytes);
	}

	MeshBufferLocation VulkanBufferCache::GetOrAllocateMeshBuffers(uInt64 meshHash, const VertexData& vertexData, bool& outFound)
	{
		auto& it = mMeshBufferLookup.Find(meshHash);

		if (it != mMeshBufferLookup.End())
		{
			outFound = true;
			return it->value;
		}

		MeshBufferLocation		bufferLocation = {};
		VulkanMultiBufferEntry	vertexEntry;
		VulkanMultiBufferEntry	indexEntry;

		const uSize vertexSizeBytes = vertexData.pVertexBuffer->Size();
		const uSize indexSizeBytes = vertexData.pIndexBuffer->Size();

		if (!mSettings.useUniqueMeshBuffers)
		{
			// Temporary, may be more buffers
			constexpr const uSize vertexIndex = 0;
			constexpr const uSize indexIndex = 0;

			VulkanMultiBuffer* pVertexBuffer = &mVertexBuffers[vertexIndex];
			VulkanMultiBuffer* pIndexBuffer = &mIndexBuffers[indexIndex];

			pVertexBuffer->Allocate<uInt8>(vertexSizeBytes, vertexEntry, nullptr);
			pIndexBuffer->Allocate<uInt8>(indexSizeBytes, indexEntry, nullptr);
			
			bufferLocation.vertexEntry		= vertexEntry;
			bufferLocation.indexEntry		= indexEntry;
			bufferLocation.pVertexBuffer	= pVertexBuffer;
			bufferLocation.pIndexBuffer		= pIndexBuffer;
		}
		else
		{
			VkBufferUsageFlags		usageFlags = 0;
			VkMemoryPropertyFlags	memoryFlags = 0;

			if (mSettings.useMeshStaging)
			{
				usageFlags	= VK_BUFFER_USAGE_TRANSFER_DST_BIT;
				memoryFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
			}
			else
			{
				usageFlags	= 0;
				memoryFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
			}

			VulkanBufferInfo vertexBufferInfo = {};
			vertexBufferInfo.sizeBytes			= vertexSizeBytes;
			vertexBufferInfo.vkBufferUsage		= VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | usageFlags;
			vertexBufferInfo.vkMemoryProperties = memoryFlags;

			VulkanMultiBuffer* pVertexBuffer = 
				&mVertexBuffers.PushBack(VulkanMultiBuffer(mpResourceManager->CreateBuffer(mpDevice, vertexBufferInfo)));

			VulkanBufferInfo indexBufferInfo = {};
			indexBufferInfo.sizeBytes			= indexSizeBytes;
			indexBufferInfo.vkBufferUsage		= VK_BUFFER_USAGE_INDEX_BUFFER_BIT | usageFlags;
			indexBufferInfo.vkMemoryProperties	= memoryFlags;

			VulkanMultiBuffer* pIndexBuffer = 
				&mIndexBuffers.PushBack(VulkanMultiBuffer(mpResourceManager->CreateBuffer(mpDevice, indexBufferInfo)));

			if (mSettings.useMeshStaging)
			{
				pVertexBuffer->Allocate<uInt8>(vertexSizeBytes, vertexEntry, nullptr);
				pIndexBuffer->Allocate<uInt8>(indexSizeBytes, indexEntry, nullptr);
			}
			else
			{
				uInt8* pVertexData;
				uInt8* pIndexData;

				pVertexBuffer->Map();
				pIndexBuffer->Map();

				pVertexBuffer->Allocate<uInt8>(vertexSizeBytes, vertexEntry, &pVertexData);
				pIndexBuffer->Allocate<uInt8>(indexSizeBytes, indexEntry, &pIndexData);

				CopyMeshData(vertexData, pVertexData, pIndexData);
			}

			bufferLocation.vertexEntry		= vertexEntry;
			bufferLocation.indexEntry		= indexEntry;
			bufferLocation.pVertexBuffer	= pVertexBuffer;
			bufferLocation.pIndexBuffer		= pIndexBuffer;
		}

		mMeshBufferLookup.Put(meshHash, bufferLocation);

		outFound = false;

		return bufferLocation;
	}

	MeshBufferLocation VulkanBufferCache::GetOrAllocateMeshStagingBuffers(uInt64 meshHash, const VertexData& vertexData, bool& outFound)
	{
		auto& it = mMeshStagingBufferLookup.Find(meshHash);

		if (it != mMeshStagingBufferLookup.End())
		{
			outFound = true;
			return it->value;
		}

		MeshBufferLocation		bufferLocation = {};
		VulkanMultiBufferEntry	vertexStagingEntry;
		VulkanMultiBufferEntry	indexStagingEntry;

		const uSize vertexSizeBytes = vertexData.pVertexBuffer->Size();
		const uSize indexSizeBytes = vertexData.pIndexBuffer->Size();

		uInt8* pVertexStagingData;
		uInt8* pIndexStagingData;

		if (!mSettings.useUniqueMeshStagingBuffers)
		{
			// Temporary, may be more buffers
			constexpr const uSize vertexStagingIndex = 0;
			constexpr const uSize indexStagingIndex = 0;

			VulkanMultiBuffer* pVertexStagingBuffer = &mVertexStagingBuffers[vertexStagingIndex];
			VulkanMultiBuffer* pIndexStagingBuffer = &mIndexStagingBuffers[indexStagingIndex];

			pVertexStagingBuffer->Map();
			pIndexStagingBuffer->Map();

			pVertexStagingBuffer->Allocate<uInt8>(vertexSizeBytes, vertexStagingEntry, &pVertexStagingData);
			pIndexStagingBuffer->Allocate<uInt8>(indexSizeBytes, indexStagingEntry, &pIndexStagingData);

			CopyMeshData(vertexData, pVertexStagingData, pIndexStagingData);

			bufferLocation.vertexEntry		= vertexStagingEntry;
			bufferLocation.indexEntry		= indexStagingEntry;
			bufferLocation.pVertexBuffer	= pVertexStagingBuffer;
			bufferLocation.pIndexBuffer		= pIndexStagingBuffer;
		}
		else
		{
			VulkanBufferInfo stagingVertexBufferInfo = {};
			stagingVertexBufferInfo.sizeBytes			= vertexSizeBytes;
			stagingVertexBufferInfo.vkBufferUsage		= VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
			stagingVertexBufferInfo.vkMemoryProperties	= VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

			VulkanMultiBuffer* pVertexStagingBuffer = 
				&mVertexStagingBuffers.PushBack(VulkanMultiBuffer(mpResourceManager->CreateBuffer(mpDevice, stagingVertexBufferInfo)));

			VulkanBufferInfo stagingIndexBufferInfo = {};
			stagingIndexBufferInfo.sizeBytes			= indexSizeBytes;
			stagingIndexBufferInfo.vkBufferUsage		= VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
			stagingIndexBufferInfo.vkMemoryProperties	= VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

			VulkanMultiBuffer* pIndexStagingBuffer = 
				&mIndexStagingBuffers.PushBack(VulkanMultiBuffer(mpResourceManager->CreateBuffer(mpDevice, stagingIndexBufferInfo)));

			pVertexStagingBuffer->Map();
			pIndexStagingBuffer->Map();

			pVertexStagingBuffer->Allocate<uInt8>(vertexSizeBytes, vertexStagingEntry, &pVertexStagingData);
			pIndexStagingBuffer->Allocate<uInt8>(indexSizeBytes, indexStagingEntry, &pIndexStagingData);

			CopyMeshData(vertexData, pVertexStagingData, pIndexStagingData);

			bufferLocation.vertexEntry		= vertexStagingEntry;
			bufferLocation.indexEntry		= indexStagingEntry;
			bufferLocation.pVertexBuffer	= pVertexStagingBuffer;
			bufferLocation.pIndexBuffer		= pIndexStagingBuffer;
		}

		mMeshStagingBufferLookup.Put(meshHash, bufferLocation);

		outFound = false;

		return bufferLocation;
	}

	PerModelBufferLocation VulkanBufferCache::AllocatePerModelBuffer(void* pPerModelData, uSize perModelSizeBytes)
	{
		VkBufferUsageFlags		usageFlags = 0;
		VkMemoryPropertyFlags	memoryFlags = 0;

		PerModelBufferLocation	bufferLocation = {};
		VulkanMultiBufferEntry	perModelEntry;

		VulkanMultiBuffer*		pPerModelBuffer;

		if (!mSettings.useUniqueMeshStagingBuffers)
		{
			// Temporary, may be more buffers
			constexpr const uSize perModelIndex = 0;

			pPerModelBuffer = &mPerModelBuffers[perModelIndex];

			pPerModelBuffer->Allocate<uInt8>(perModelSizeBytes, perModelEntry, nullptr);

			bufferLocation.perModelEntry	= perModelEntry;
			bufferLocation.pPerModelBuffer	= pPerModelBuffer;
		}
		else
		{
			if (mSettings.useUniformStaging)
			{
				usageFlags	= VK_BUFFER_USAGE_TRANSFER_DST_BIT;
				memoryFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
			}
			else
			{
				usageFlags	= 0;
				memoryFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
			}

			VulkanBufferInfo perModelBufferInfo = {};
			perModelBufferInfo.sizeBytes			= perModelSizeBytes;
			perModelBufferInfo.vkBufferUsage		= VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | usageFlags;
			perModelBufferInfo.vkMemoryProperties	= memoryFlags;

			pPerModelBuffer = &mPerModelBuffers.PushBack(VulkanMultiBuffer(mpResourceManager->CreateBuffer(mpDevice, perModelBufferInfo)));

			if (mSettings.useUniformStaging)
			{
				pPerModelBuffer->Allocate<uInt8>(perModelSizeBytes, perModelEntry, nullptr);
			}
			else
			{
				uInt8* pPerModelBufferData;

				pPerModelBuffer->Map();
				pPerModelBuffer->Allocate<uInt8>(perModelSizeBytes, perModelEntry, &pPerModelBufferData);

				memcpy_s(pPerModelBufferData, perModelSizeBytes, pPerModelData, perModelSizeBytes);
			}

			bufferLocation.perModelEntry	= perModelEntry;
			bufferLocation.pPerModelBuffer	= pPerModelBuffer;
		}

		return bufferLocation;
	}

	PerModelBufferLocation VulkanBufferCache::AllocatePerModelStagingBuffer(void* pPerModelData, uSize perModelSizeBytes)
	{
		PerModelBufferLocation	bufferLocation = {};
		VulkanMultiBufferEntry	perModelStagingEntry;
		uInt8*					pPerModelStagingData;

		if (!mSettings.useUniqueUniformStagingBuffers)
		{
			// Temporary, may be more buffers
			constexpr const uSize perModelStagingIndex = 0;

			VulkanMultiBuffer* pPerModelStagingBuffer = &mPerModelStagingBuffers[perModelStagingIndex];

			pPerModelStagingBuffer->Map();
			pPerModelStagingBuffer->Allocate<uInt8>(perModelSizeBytes, perModelStagingEntry, &pPerModelStagingData);

			memcpy_s(pPerModelStagingData, perModelSizeBytes, pPerModelData, perModelSizeBytes);

			bufferLocation.perModelEntry	= perModelStagingEntry;
			bufferLocation.pPerModelBuffer	= pPerModelStagingBuffer;
		}
		else
		{
			VulkanBufferInfo perModelStagingBufferInfo = {};
			perModelStagingBufferInfo.sizeBytes				= perModelSizeBytes;
			perModelStagingBufferInfo.vkBufferUsage			= VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
			perModelStagingBufferInfo.vkMemoryProperties	= VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

			VulkanMultiBuffer* pPerModelStagingBuffer = 
				&mPerModelStagingBuffers.PushBack(VulkanMultiBuffer(mpResourceManager->CreateBuffer(mpDevice, perModelStagingBufferInfo)));

			pPerModelStagingBuffer->Map();
			pPerModelStagingBuffer->Allocate<uInt8>(perModelSizeBytes, perModelStagingEntry, &pPerModelStagingData);

			memcpy_s(pPerModelStagingData, perModelSizeBytes, pPerModelData, perModelSizeBytes);

			bufferLocation.perModelEntry	= perModelStagingEntry;
			bufferLocation.pPerModelBuffer	= pPerModelStagingBuffer;
		}

		return bufferLocation;
	}

	void VulkanBufferCache::ResetPerModelBuffers()
	{
		if (!mSettings.usePerModelPushConstants)
		{
			if (mSettings.useUniqueUniformBuffers)
			{
				for (VulkanMultiBuffer& buffer : mPerModelBuffers)
				{
					buffer.Unmap();
					mpResourceManager->DestroyBuffer(buffer.GetVulkanBuffer());
				}

				mPerModelBuffers.Clear();

				if (mSettings.useUniformStaging)
				{
					for (VulkanMultiBuffer& buffer : mPerModelStagingBuffers)
					{
						buffer.Unmap();
						mpResourceManager->DestroyBuffer(buffer.GetVulkanBuffer());
					}

					mPerModelStagingBuffers.Clear();
				}
			}
			else
			{
				mPerModelBuffers[0].FreeAll();

				if (mSettings.useUniformStaging)
				{
					mPerModelStagingBuffers[0].FreeAll();
				}
			}
		}
	}

	void VulkanBufferCache::FillRenderableVertexData(VulkanRenderable& renderable, uInt64 meshHash, const VertexData& vertexData, bool& outFound)
	{
		bool meshBufferFound;

		MeshBufferLocation meshBufferLocation = GetOrAllocateMeshBuffers(meshHash, vertexData, meshBufferFound);

		if (mSettings.useMeshStaging)
		{
			bool meshStagingBufferFound;

			MeshBufferLocation meshStagingBufferLocation = GetOrAllocateMeshStagingBuffers(meshHash, vertexData, meshStagingBufferFound);

			if (!meshBufferFound)
			{
				TransferCommand vertexTransfer = {};
				vertexTransfer.pSrcBuffer	= meshStagingBufferLocation.pVertexBuffer;
				vertexTransfer.pDestBuffer	= meshBufferLocation.pVertexBuffer;
				vertexTransfer.srcEntry		= meshStagingBufferLocation.vertexEntry;
				vertexTransfer.destEntry	= meshBufferLocation.vertexEntry;

				mReadyTransfers.PushBack(vertexTransfer);

				TransferCommand indexTransfer = {};
				indexTransfer.pSrcBuffer	= meshStagingBufferLocation.pIndexBuffer;
				indexTransfer.pDestBuffer	= meshBufferLocation.pIndexBuffer;
				indexTransfer.srcEntry		= meshStagingBufferLocation.indexEntry;
				indexTransfer.destEntry		= meshBufferLocation.indexEntry;

				mReadyTransfers.PushBack(indexTransfer);
			}
		}

		renderable.meshLocation = meshBufferLocation;
		renderable.indexStart	= meshBufferLocation.indexEntry.offset / sizeof(float); // @TODO?
		renderable.indexCount	= meshBufferLocation.indexEntry.sizeBytes / vertexData.index.Size();
	}

	void VulkanBufferCache::FillRenderablePerModelData(VulkanRenderable& renderable, uInt64 renderableId, void* pPerModelData, uSize perModelSizeBytes)
	{
		PerModelBufferLocation perModelBufferLocation = AllocatePerModelBuffer(pPerModelData, perModelSizeBytes);

		// @TODO: This is using a copy command for each object when one is sufficient for ubos

		if (mSettings.useUniformStaging && !mSettings.usePerModelPushConstants)
		{
			PerModelBufferLocation perModelBufferStagingLocation = AllocatePerModelStagingBuffer(pPerModelData, perModelSizeBytes);

			TransferCommand perModelTransfer = {};
			perModelTransfer.pSrcBuffer		= perModelBufferStagingLocation.pPerModelBuffer;
			perModelTransfer.pDestBuffer	= perModelBufferLocation.pPerModelBuffer;
			perModelTransfer.srcEntry		= perModelBufferStagingLocation.perModelEntry;
			perModelTransfer.destEntry		= perModelBufferLocation.perModelEntry;

			mReadyTransfers.PushBack(perModelTransfer);
		}

		renderable.perModelLocation = perModelBufferLocation;
	}

	void VulkanBufferCache::RecordTransfers(VulkanCommandRecorder& recorder)
	{
		while (!mReadyTransfers.IsEmpty())
		{
			TransferCommand transfer = mReadyTransfers.PopFront();

			recorder.CopyBuffer(
				transfer.pSrcBuffer->GetVulkanBuffer(), transfer.pDestBuffer->GetVulkanBuffer(), 
				transfer.srcEntry.sizeBytes, transfer.srcEntry.offset, transfer.destEntry.offset);
		}

		mReadyTransfers.Clear();
	}
}