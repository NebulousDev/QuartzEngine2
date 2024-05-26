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
			for (uSize i = 0; i < 8; i++)
			{
				VulkanBufferInfo vertexBufferInfo = {};
				vertexBufferInfo.sizeBytes			= mSettings.vertexBufferSizeMb * (1024 * 1024);
				vertexBufferInfo.vkBufferUsage		= VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | usageFlags;
				vertexBufferInfo.vkMemoryProperties = memoryFlags;

				mVertexBuffers.PushBack(VulkanMultiBuffer(mpResourceManager->CreateBuffer(mpDevice, vertexBufferInfo)));
			}

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
			perModelBufferInfo.sizeBytes			= mSettings.uniformBufferSizeMb * (1024 * 1024);
			perModelBufferInfo.vkBufferUsage		= VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | usageFlags;
			perModelBufferInfo.vkMemoryProperties	= memoryFlags;

			for (uSize i = 0; i < mSettings.maxUniformSets; i++)
			{
				mUniformBuffers.PushBack(VulkanMultiBuffer(mpResourceManager->CreateBuffer(mpDevice, perModelBufferInfo)));
			}
		}

		if (!mSettings.useUniqueMeshStagingBuffers)
		{
			if (mSettings.useMeshStaging)
			{
				for (uSize i = 0; i < 8; i++)
				{
					VulkanBufferInfo stagingVertexBufferInfo = {};
					stagingVertexBufferInfo.sizeBytes			= mSettings.vertexBufferSizeMb * (1024 * 1024);
					stagingVertexBufferInfo.vkBufferUsage		= VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
					stagingVertexBufferInfo.vkMemoryProperties	= VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

					mVertexStagingBuffers.PushBack(VulkanMultiBuffer(mpResourceManager->CreateBuffer(mpDevice, stagingVertexBufferInfo)));
				}

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
				stagingPerModelBufferInfo.sizeBytes				= mSettings.uniformBufferSizeMb * (1024 * 1024);
				stagingPerModelBufferInfo.vkBufferUsage			= VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
				stagingPerModelBufferInfo.vkMemoryProperties	= VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

				for (uSize i = 0; i < mSettings.maxUniformSets; i++)
				{
					mUniformStagingBuffers.PushBack(VulkanMultiBuffer(mpResourceManager->CreateBuffer(mpDevice, stagingPerModelBufferInfo)));
					mUniformStagingBuffers[i].Map();
				}
			}
		}
	}

	void VulkanBufferCache::Initialize(VulkanDevice* pDevice, VulkanResourceManager* pResourceManager, 
		const VulkanRenderSettings& settings)
	{
		mSettings			= settings;
		mpResourceManager	= pResourceManager;
		mpDevice			= pDevice;

		InitializeDefaultBuffers();
	}

	void CopyMeshVertexData(const VertexStream& vertexStream, void* pOutVertexData, uInt32 vertexPadBytes)
	{
		const void* pVertexData			= vertexStream.pVertexBuffer->Data();
		const uSize verticesSizeBytes	= vertexStream.pVertexBuffer->Size();

		memcpy_s(pOutVertexData, verticesSizeBytes, pVertexData, verticesSizeBytes);
		memset((uInt8*)pOutVertexData + verticesSizeBytes, 0, vertexPadBytes);
	}

	void CopyMeshIndexData(const IndexStream& indexStream, void* pOutIndexData, uInt32 indexPadBytes)
	{
		const void* pIndexData			= indexStream.pIndexBuffer->Data();
		const uSize indicesSizeBytes	= indexStream.pIndexBuffer->Size();

		memcpy_s(pOutIndexData, indicesSizeBytes, pIndexData, indicesSizeBytes);
		memset((uInt8*)pOutIndexData + indicesSizeBytes, 0, indexPadBytes);
	}

	void CopyPerModelData(void* pUniformData, void* pOutPerModelBuffer, uSize sizeBytes)
	{
		memcpy_s(pOutPerModelBuffer, sizeBytes, pUniformData, sizeBytes);
	}

	bool VulkanBufferCache::GetOrAllocateMeshBuffers(const Model& model, 
		MeshBufferLocation& outBufferLocation, 
		uSize vertexAlignBytes, uSize indexAlignBytes, bool& outFound)
	{
		auto& it = mMeshBufferLookup.Find(model.GetAssetID());

		if (it != mMeshBufferLookup.End())
		{
			outFound = true;
			outBufferLocation = it->value;
			return true;
		}

		MeshBufferLocation bufferLocation = {};
		
		for (const VertexStream& vertexStream : model.vertexStreams)
		{
			if (!vertexStream.pVertexBuffer)
			{
				// Empty streamIdx, continue
				continue;
			}

			VulkanMultiBufferEntry	vertexEntry;
			uInt8*					pVertexData;

			uSize verticesSizeBytes		= vertexStream.pVertexBuffer->Size();
			uInt32 vertexAlignmentDiff	= verticesSizeBytes % vertexAlignBytes;
			//verticesSizeBytes			+= vertexAlignBytes - vertexAlignmentDiff;

			if (!mSettings.useUniqueMeshBuffers)
			{
				VulkanMultiBuffer* pVertexBuffer = &mVertexBuffers[vertexStream.streamIdx];

				pVertexBuffer->Allocate<uInt8>(verticesSizeBytes, vertexEntry, nullptr);
			
				bufferLocation.vertexEntries.PushBack(vertexEntry);
				bufferLocation.vertexBuffers.PushBack(pVertexBuffer);
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
				vertexBufferInfo.sizeBytes			= verticesSizeBytes;
				vertexBufferInfo.vkBufferUsage		= VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | usageFlags;
				vertexBufferInfo.vkMemoryProperties = memoryFlags;

				VulkanMultiBuffer* pVertexBuffer = 
					&mVertexBuffers.PushBack(VulkanMultiBuffer(mpResourceManager->CreateBuffer(mpDevice, vertexBufferInfo)));

				if (mSettings.useMeshStaging)
				{
					pVertexBuffer->Allocate<uInt8>(verticesSizeBytes, vertexEntry, nullptr);
				}
				else
				{
					pVertexBuffer->Map();
					pVertexBuffer->Allocate<uInt8>(verticesSizeBytes, vertexEntry, &pVertexData);

					CopyMeshVertexData(vertexStream, pVertexData, vertexAlignmentDiff);
				}

				bufferLocation.vertexEntries.PushBack(vertexEntry);
				bufferLocation.vertexBuffers.PushBack(pVertexBuffer);
			}
		}

		const IndexStream& indexStream = model.indexStream;

		VulkanMultiBufferEntry	indexEntry;
		uInt8*					pIndexData;

		uSize indicesSizeBytes		= indexStream.pIndexBuffer->Size();
		uInt32 indexAlignmentDiff	= indicesSizeBytes % indexAlignBytes;
		//indicesSizeBytes			+= indexAlignBytes - indexAlignmentDiff;

		if (!mSettings.useUniqueMeshBuffers)
		{
			constexpr const uSize indexIndex = 0; // TODO: unique index buffers?

			VulkanMultiBuffer* pIndexBuffer = &mIndexBuffers[indexIndex];

			pIndexBuffer->Allocate<uInt8>(indicesSizeBytes, indexEntry, nullptr);
			
			bufferLocation.indexEntry	= indexEntry;
			bufferLocation.pIndexBuffer	= pIndexBuffer;
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

			VulkanBufferInfo indexBufferInfo = {};
			indexBufferInfo.sizeBytes			= indicesSizeBytes;
			indexBufferInfo.vkBufferUsage		= VK_BUFFER_USAGE_INDEX_BUFFER_BIT | usageFlags;
			indexBufferInfo.vkMemoryProperties	= memoryFlags;

			VulkanMultiBuffer* pIndexBuffer = 
				&mIndexBuffers.PushBack(VulkanMultiBuffer(mpResourceManager->CreateBuffer(mpDevice, indexBufferInfo)));

			if (mSettings.useMeshStaging)
			{
				pIndexBuffer->Allocate<uInt8>(indicesSizeBytes, indexEntry, nullptr);
			}
			else
			{
				pIndexBuffer->Map();
				pIndexBuffer->Allocate<uInt8>(indicesSizeBytes, indexEntry, &pIndexData);

				CopyMeshIndexData(indexStream, pIndexData, indexAlignmentDiff);
			}

			bufferLocation.indexEntry	= indexEntry;
			bufferLocation.pIndexBuffer	= pIndexBuffer;
		}

		mMeshBufferLookup.Put(model.GetAssetID(), bufferLocation);

		outFound = false;
		outBufferLocation = bufferLocation;

		return true;
	}

	bool VulkanBufferCache::GetOrAllocateMeshStagingBuffers(const Model& model, 
		MeshBufferLocation& outStagingBufferLocation, 
		uSize vertexAlignBytes, uSize indexAlignBytes, bool& outFound)
	{
		auto& it = mMeshStagingBufferLookup.Find(model.GetAssetID());

		if (it != mMeshStagingBufferLookup.End())
		{
			outFound = true;
			outStagingBufferLocation = it->value;
			return true;
		}

		MeshBufferLocation stagingBufferLocation = {};
		
		for (const VertexStream& vertexStream : model.vertexStreams)
		{
			if (!vertexStream.pVertexBuffer)
			{
				// Empty streamIdx, continue
				continue;
			}

			VulkanMultiBufferEntry	stagingVertexEntry;
			uInt8*					pStagingVertexData;

			uSize verticesSizeBytes		= vertexStream.pVertexBuffer->Size();
			uInt32 vertexAlignmentDiff	= verticesSizeBytes % vertexAlignBytes;
			//verticesSizeBytes			+= vertexAlignBytes - vertexAlignmentDiff;

			if (!mSettings.useUniqueMeshStagingBuffers)
			{
				VulkanMultiBuffer* pVertexStagingBuffer = &mVertexStagingBuffers[vertexStream.streamIdx];

				pVertexStagingBuffer->Map();
				pVertexStagingBuffer->Allocate<uInt8>(verticesSizeBytes, stagingVertexEntry, &pStagingVertexData);

				CopyMeshVertexData(vertexStream, pStagingVertexData, vertexAlignmentDiff);

				stagingBufferLocation.vertexEntries.PushBack(stagingVertexEntry);
				stagingBufferLocation.vertexBuffers.PushBack(pVertexStagingBuffer);
			}
			else
			{
				VulkanBufferInfo stagingVertexBufferInfo = {};
				stagingVertexBufferInfo.sizeBytes			= verticesSizeBytes;
				stagingVertexBufferInfo.vkBufferUsage		= VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
				stagingVertexBufferInfo.vkMemoryProperties	= VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

				VulkanMultiBuffer* pVertexStagingBuffer = 
					&mVertexStagingBuffers.PushBack(VulkanMultiBuffer(mpResourceManager->CreateBuffer(mpDevice, stagingVertexBufferInfo)));

				pVertexStagingBuffer->Map();
				pVertexStagingBuffer->Allocate<uInt8>(verticesSizeBytes, stagingVertexEntry, &pStagingVertexData);

				CopyMeshVertexData(vertexStream, pStagingVertexData, vertexAlignmentDiff);

				stagingBufferLocation.vertexEntries.PushBack(stagingVertexEntry);
				stagingBufferLocation.vertexBuffers.PushBack(pVertexStagingBuffer);
			}
		}

		const IndexStream& indexStream = model.indexStream;

		VulkanMultiBufferEntry	stagingIndexEntry;
		uInt8*					pStagingIndexData;

		uSize indicesSizeBytes		= indexStream.pIndexBuffer->Size();
		uInt32 indexAlignmentDiff	= indicesSizeBytes % indexAlignBytes;
		//indicesSizeBytes			+= indexAlignBytes - indexAlignmentDiff;

		if (!mSettings.useUniqueMeshStagingBuffers)
		{
			constexpr const uSize indexStagingIndex = 0;  // TODO: unique index buffers?

			VulkanMultiBuffer* pIndexStagingBuffer = &mIndexStagingBuffers[indexStagingIndex];

			pIndexStagingBuffer->Map();
			pIndexStagingBuffer->Allocate<uInt8>(indicesSizeBytes, stagingIndexEntry, &pStagingIndexData);

			CopyMeshIndexData(indexStream, pStagingIndexData, indexAlignmentDiff);

			stagingBufferLocation.indexEntry		= stagingIndexEntry;
			stagingBufferLocation.pIndexBuffer		= pIndexStagingBuffer;
		}
		else
		{
			VulkanBufferInfo stagingIndexBufferInfo = {};
			stagingIndexBufferInfo.sizeBytes			= indicesSizeBytes;
			stagingIndexBufferInfo.vkBufferUsage		= VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
			stagingIndexBufferInfo.vkMemoryProperties	= VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

			VulkanMultiBuffer* pIndexStagingBuffer = 
				&mIndexStagingBuffers.PushBack(VulkanMultiBuffer(mpResourceManager->CreateBuffer(mpDevice, stagingIndexBufferInfo)));

			pIndexStagingBuffer->Map();
			pIndexStagingBuffer->Allocate<uInt8>(indicesSizeBytes, stagingIndexEntry, &pStagingIndexData);

			CopyMeshIndexData(indexStream, pStagingIndexData, indexAlignmentDiff);

			stagingBufferLocation.indexEntry = stagingIndexEntry;
			stagingBufferLocation.pIndexBuffer = pIndexStagingBuffer;
		}

		mMeshStagingBufferLookup.Put(model.GetAssetID(), stagingBufferLocation);

		outFound = false;
		outStagingBufferLocation = stagingBufferLocation;

		return true;
	}

	bool VulkanBufferCache::AllocateUniformBuffer(UniformBufferLocation& outUniformBuffer, uSize set, void* pUniformData, uSize uniformSizeBytes)
	{
		VkBufferUsageFlags		usageFlags = 0;
		VkMemoryPropertyFlags	memoryFlags = 0;

		UniformBufferLocation	bufferLocation = {};
		VulkanMultiBufferEntry	entry;

		VulkanMultiBuffer*		pBuffer;

		if (!mSettings.useUniqueMeshStagingBuffers)
		{
			const uSize uniformIndex = set;

			pBuffer = &mUniformBuffers[uniformIndex];

			pBuffer->Allocate<uInt8>(uniformSizeBytes, entry, nullptr);

			bufferLocation.entry	= entry;
			bufferLocation.pBuffer	= pBuffer;
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
			perModelBufferInfo.sizeBytes			= uniformSizeBytes;
			perModelBufferInfo.vkBufferUsage		= VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | usageFlags;
			perModelBufferInfo.vkMemoryProperties	= memoryFlags;

			pBuffer = &mUniformBuffers.PushBack(VulkanMultiBuffer(mpResourceManager->CreateBuffer(mpDevice, perModelBufferInfo)));

			if (mSettings.useUniformStaging)
			{
				pBuffer->Allocate<uInt8>(uniformSizeBytes, entry, nullptr);
			}
			else
			{
				uInt8* pPerModelBufferData;

				pBuffer->Map();
				pBuffer->Allocate<uInt8>(uniformSizeBytes, entry, &pPerModelBufferData);

				memcpy_s(pPerModelBufferData, uniformSizeBytes, pUniformData, uniformSizeBytes);
			}

			bufferLocation.entry	= entry;
			bufferLocation.pBuffer	= pBuffer;
		}

		outUniformBuffer = bufferLocation;

		return true;
	}

	bool VulkanBufferCache::AllocateUniformStagingBuffer(UniformBufferLocation& outUniformBuffer, uSize set, void* pUniformData, uSize uniformSizeBytes)
	{
		UniformBufferLocation	bufferLocation = {};
		VulkanMultiBufferEntry	uniformStagingEntry;
		uInt8*					pUniformStagingData;

		if (!mSettings.useUniqueUniformStagingBuffers)
		{
			const uSize uniformStagingIndex = set;

			VulkanMultiBuffer* pUniformStagingBuffer = &mUniformStagingBuffers[uniformStagingIndex];

			pUniformStagingBuffer->Map();
			pUniformStagingBuffer->Allocate<uInt8>(uniformSizeBytes, uniformStagingEntry, &pUniformStagingData);

			memcpy_s(pUniformStagingData, uniformSizeBytes, pUniformData, uniformSizeBytes);

			bufferLocation.entry	= uniformStagingEntry;
			bufferLocation.pBuffer	= pUniformStagingBuffer;
		}
		else
		{
			VulkanBufferInfo uniformStagingBufferInfo = {};
			uniformStagingBufferInfo.sizeBytes				= uniformSizeBytes;
			uniformStagingBufferInfo.vkBufferUsage			= VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
			uniformStagingBufferInfo.vkMemoryProperties		= VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

			VulkanMultiBuffer* pPerModelStagingBuffer = 
				&mUniformStagingBuffers.PushBack(VulkanMultiBuffer(mpResourceManager->CreateBuffer(mpDevice, uniformStagingBufferInfo)));

			pPerModelStagingBuffer->Map();
			pPerModelStagingBuffer->Allocate<uInt8>(uniformSizeBytes, uniformStagingEntry, &pUniformStagingData);

			memcpy_s(pUniformStagingData, uniformSizeBytes, pUniformData, uniformSizeBytes);

			bufferLocation.entry	= uniformStagingEntry;
			bufferLocation.pBuffer	= pPerModelStagingBuffer;
		}

		outUniformBuffer = bufferLocation;

		return true;
	}

	void VulkanBufferCache::ResetPerModelBuffers()
	{
		if (!mSettings.usePerModelPushConstants)
		{
			if (mSettings.useUniqueUniformBuffers)
			{
				for (VulkanMultiBuffer& buffer : mUniformBuffers)
				{
					buffer.Unmap();
					mpResourceManager->DestroyBuffer(buffer.GetVulkanBuffer());
				}

				mUniformBuffers.Clear();

				if (mSettings.useUniformStaging)
				{
					for (VulkanMultiBuffer& buffer : mUniformStagingBuffers)
					{
						buffer.Unmap();
						mpResourceManager->DestroyBuffer(buffer.GetVulkanBuffer());
					}

					mUniformStagingBuffers.Clear();
				}
			}
			else
			{
				mUniformBuffers[0].FreeAll();

				if (mSettings.useUniformStaging)
				{
					mUniformStagingBuffers[0].FreeAll();
				}
			}
		}
	}

	bool VulkanBufferCache::GetOrAllocateBuffers(const Model& model,
		MeshBufferLocation& outbufferLocation, MeshBufferLocation& outStagingbufferLocation,
		uSize vertexAlignBytes, uSize indexAlignBytes, bool& outFound)
	{
		bool meshBufferFound;
		MeshBufferLocation meshBufferLocation;
		if (!GetOrAllocateMeshBuffers(model, meshBufferLocation, 
			vertexAlignBytes, indexAlignBytes, meshBufferFound))
		{
			outFound = false;
			return false;
		}
		else
		{
			outFound = true;
			outbufferLocation = meshBufferLocation;
		}

		if (mSettings.useMeshStaging)
		{
			bool meshStagingBufferFound;
			MeshBufferLocation meshStagingBufferLocation;
			if (!GetOrAllocateMeshStagingBuffers(model, meshStagingBufferLocation, 
				vertexAlignBytes, indexAlignBytes, meshStagingBufferFound))
			{
				outFound = false;
				return false;
			}
			else
			{
				outStagingbufferLocation = meshStagingBufferLocation;
			}

			if (!meshBufferFound)
			{
				for (uSize i = 0; i < meshStagingBufferLocation.vertexBuffers.Size(); i++)
				{
					TransferCommand vertexTransfer = {};
					vertexTransfer.pSrcBuffer	= meshStagingBufferLocation.vertexBuffers[i];
					vertexTransfer.pDestBuffer	= meshBufferLocation.vertexBuffers[i];
					vertexTransfer.srcEntry		= meshStagingBufferLocation.vertexEntries[i];
					vertexTransfer.destEntry	= meshBufferLocation.vertexEntries[i];

					mReadyTransfers.PushBack(vertexTransfer);
				}

				TransferCommand indexTransfer = {};
				indexTransfer.pSrcBuffer	= meshStagingBufferLocation.pIndexBuffer;
				indexTransfer.pDestBuffer	= meshBufferLocation.pIndexBuffer;
				indexTransfer.srcEntry		= meshStagingBufferLocation.indexEntry;
				indexTransfer.destEntry		= meshBufferLocation.indexEntry;

				mReadyTransfers.PushBack(indexTransfer);
			}
		}

		return true;
	}

	bool VulkanBufferCache::AllocateAndWriteUniformData(UniformBufferLocation& outUniformBuffer, uSize set, void* pUniformData, uSize uniformSizeBytes)
	{
		UniformBufferLocation uniformBufferLocation;

		if (!AllocateUniformBuffer(uniformBufferLocation, set, pUniformData, uniformSizeBytes))
		{
			return false;
		}

		// @TODO: This is using a copy command for each object when one is sufficient for ubos

		if (mSettings.useUniformStaging && !mSettings.usePerModelPushConstants)
		{
			UniformBufferLocation uniformBufferStagingLocation;

			if (!AllocateUniformStagingBuffer(uniformBufferStagingLocation, set, pUniformData, uniformSizeBytes))
			{
				return false;
			}

			TransferCommand perModelTransfer = {};
			perModelTransfer.pSrcBuffer		= uniformBufferStagingLocation.pBuffer;
			perModelTransfer.pDestBuffer	= uniformBufferLocation.pBuffer;
			perModelTransfer.srcEntry		= uniformBufferStagingLocation.entry;
			perModelTransfer.destEntry		= uniformBufferLocation.entry;

			mReadyTransfers.PushBack(perModelTransfer);
		}

		outUniformBuffer = uniformBufferLocation;

		return true;
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