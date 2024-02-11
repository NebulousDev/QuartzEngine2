#include "Vulkan/VulkanRenderScene.h"

#include "Log.h"

namespace Quartz
{
	void VulkanRenderScene::Initialize(VulkanDevice* pDevice, VulkanResourceManager* pResourceManager, const VulkanRenderSettings& settings)
	{
		mSettings			= settings;
		mpResourceManager	= pResourceManager;
		mpDevice			= pDevice;

		VkBufferUsageFlags		usageFlags = 0;
		VkMemoryPropertyFlags	memoryFlags = 0;

		if (settings.useMeshStaging)
		{
			usageFlags	= VK_BUFFER_USAGE_TRANSFER_DST_BIT;
			memoryFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
		}
		else
		{
			usageFlags	= 0;
			memoryFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
		}

		if (!settings.useUniqueMeshBuffers)
		{
			VulkanBufferInfo vertexBufferInfo = {};
			vertexBufferInfo.sizeBytes			= settings.vertexBufferSizeMb * (1024 * 1024);
			vertexBufferInfo.vkBufferUsage		= VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | usageFlags;
			vertexBufferInfo.vkMemoryProperties = memoryFlags;

			mVertexBuffers.PushBack(VulkanMultiBuffer(pResourceManager->CreateBuffer(pDevice, vertexBufferInfo)));

			VulkanBufferInfo indexBufferInfo = {};
			indexBufferInfo.sizeBytes			= settings.indexBufferSizeMb * (1024 * 1024);
			indexBufferInfo.vkBufferUsage		= VK_BUFFER_USAGE_INDEX_BUFFER_BIT | usageFlags;
			indexBufferInfo.vkMemoryProperties	= memoryFlags;

			mIndexBuffers.PushBack(VulkanMultiBuffer(pResourceManager->CreateBuffer(pDevice, indexBufferInfo)));
		}

		VulkanBufferInfo globalBufferInfo = {};
		globalBufferInfo.sizeBytes				= settings.globalBufferSizeBytes;
		globalBufferInfo.vkBufferUsage			= VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | usageFlags;
		globalBufferInfo.vkMemoryProperties		= memoryFlags;

		mGlobalBuffer = pResourceManager->CreateBuffer(pDevice, globalBufferInfo);

		if (!settings.useUniqueUniformBuffers)
		{
			VulkanBufferInfo perInstanceBufferInfo = {};
			perInstanceBufferInfo.sizeBytes				= settings.perInstanceBufferSizeMb * (1024 * 1024);
			perInstanceBufferInfo.vkBufferUsage			= VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | usageFlags;
			perInstanceBufferInfo.vkMemoryProperties	= memoryFlags;

			mPerInstanceBuffers.PushBack(VulkanMultiBuffer(pResourceManager->CreateBuffer(pDevice, perInstanceBufferInfo)));

			VulkanBufferInfo perModelBufferInfo = {};
			perModelBufferInfo.sizeBytes			= settings.perModelBufferSizeMb * (1024 * 1024);
			perModelBufferInfo.vkBufferUsage		= VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | usageFlags;
			perModelBufferInfo.vkMemoryProperties	= memoryFlags;

			mPerModelBuffers.PushBack(VulkanMultiBuffer(pResourceManager->CreateBuffer(pDevice, perModelBufferInfo)));
		}

		if (!settings.useUniqueMeshStagingBuffers)
		{
			if (settings.useMeshStaging)
			{
				VulkanBufferInfo stagingVertexBufferInfo = {};
				stagingVertexBufferInfo.sizeBytes			= settings.vertexBufferSizeMb * (1024 * 1024);
				stagingVertexBufferInfo.vkBufferUsage		= VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
				stagingVertexBufferInfo.vkMemoryProperties	= VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

				mVertexStagingBuffers.PushBack(VulkanMultiBuffer(pResourceManager->CreateBuffer(pDevice, stagingVertexBufferInfo)));

				VulkanBufferInfo stagingGlobalBufferInfo = {};
				stagingGlobalBufferInfo.sizeBytes			= settings.perInstanceBufferSizeMb * (1024 * 1024);
				stagingGlobalBufferInfo.vkBufferUsage		= VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
				stagingGlobalBufferInfo.vkMemoryProperties	= VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

				mGlobalStagingBuffer = pResourceManager->CreateBuffer(pDevice, stagingGlobalBufferInfo);

				VulkanBufferInfo stagingIndexBufferInfo = {};
				stagingIndexBufferInfo.sizeBytes			= settings.indexBufferSizeMb * (1024 * 1024);
				stagingIndexBufferInfo.vkBufferUsage		= VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
				stagingIndexBufferInfo.vkMemoryProperties	= VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

				mIndexStagingBuffers.PushBack(VulkanMultiBuffer(pResourceManager->CreateBuffer(pDevice, stagingIndexBufferInfo)));

				VulkanBufferInfo stagingPerInstanceBufferInfo = {};
				stagingPerInstanceBufferInfo.sizeBytes			= settings.perInstanceBufferSizeMb * (1024 * 1024);
				stagingPerInstanceBufferInfo.vkBufferUsage		= VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
				stagingPerInstanceBufferInfo.vkMemoryProperties	= VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

				mPerInstanceStagingBuffers.PushBack(VulkanMultiBuffer(pResourceManager->CreateBuffer(pDevice, stagingPerInstanceBufferInfo)));

				mVertexStagingBuffers[0].Map();
				mIndexStagingBuffers[0].Map();
				mGlobalStagingBuffer.Map();
				mPerInstanceStagingBuffers[0].Map();
			}

			if (settings.useUniformStaging && !settings.usePerModelPushConstants)
			{
				VulkanBufferInfo stagingPerModelBufferInfo = {};
				stagingPerModelBufferInfo.sizeBytes				= settings.perModelBufferSizeMb * (1024 * 1024);
				stagingPerModelBufferInfo.vkBufferUsage			= VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
				stagingPerModelBufferInfo.vkMemoryProperties	= VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

				mPerModelStagingBuffers.PushBack(VulkanMultiBuffer(pResourceManager->CreateBuffer(pDevice, stagingPerModelBufferInfo)));

				mPerModelStagingBuffers[0].Map();

			}
		}
	}

	void CopyMeshData(const ModelData& data, float* pVertData, uInt16* pIndexData)
	{
		memcpy_s(pVertData, data.vertices.Size() * sizeof(float), data.vertices.Data(), data.vertices.Size() * sizeof(float));
		memcpy_s(pIndexData, data.indices.Size() * sizeof(uInt16), data.indices.Data(), data.indices.Size() * sizeof(uInt16));
	}

	void CopyPerModelData(uInt8* pPerModelData, uInt8* pPerModelBuffer, uSize sizeBytes)
	{
		memcpy_s(pPerModelBuffer, sizeBytes, pPerModelData, sizeBytes);
	}

	MeshBufferLocation VulkanRenderScene::GetOrAllocateMeshBuffers(MeshComponent& renderable, bool& outFound)
	{
		auto& it = mMeshBufferLookup.Find(renderable.modelURIHash);

		if (it != mMeshBufferLookup.End())
		{
			outFound = true;
			return it->value;
		}

		MeshBufferLocation		bufferLocation = {};
		VulkanMultiBufferEntry	vertexEntry;
		VulkanMultiBufferEntry	indexEntry;

		if (!mSettings.useUniqueMeshBuffers)
		{
			// Temporary, may be more buffers
			constexpr const uSize vertexIndex = 0;
			constexpr const uSize indexIndex = 0;

			VulkanMultiBuffer* pVertexBuffer = &mVertexBuffers[vertexIndex];
			VulkanMultiBuffer* pIndexBuffer = &mIndexBuffers[indexIndex];

			pVertexBuffer->Allocate<float>(renderable.modelData.vertices.Size(), vertexEntry, nullptr);
			pIndexBuffer->Allocate<uInt16>(renderable.modelData.indices.Size(), indexEntry, nullptr);
			
			bufferLocation.vertexEntry			= vertexEntry;
			bufferLocation.indexEntry			= indexEntry;
			bufferLocation.vertexBufferIndex	= vertexIndex;
			bufferLocation.indexBufferIndex		= indexIndex;
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
			vertexBufferInfo.sizeBytes			= renderable.modelData.vertices.Size() * sizeof(float);
			vertexBufferInfo.vkBufferUsage		= VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | usageFlags;
			vertexBufferInfo.vkMemoryProperties = memoryFlags;

			VulkanMultiBuffer* pVertexBuffer = 
				&mVertexBuffers.PushBack(VulkanMultiBuffer(mpResourceManager->CreateBuffer(mpDevice, vertexBufferInfo)));

			VulkanBufferInfo indexBufferInfo = {};
			indexBufferInfo.sizeBytes			= renderable.modelData.indices.Size() * sizeof(uInt16);
			indexBufferInfo.vkBufferUsage		= VK_BUFFER_USAGE_INDEX_BUFFER_BIT | usageFlags;
			indexBufferInfo.vkMemoryProperties	= memoryFlags;

			VulkanMultiBuffer* pIndexBuffer = 
				&mIndexBuffers.PushBack(VulkanMultiBuffer(mpResourceManager->CreateBuffer(mpDevice, indexBufferInfo)));

			if (mSettings.useMeshStaging)
			{
				pVertexBuffer->Allocate<float>(renderable.modelData.vertices.Size(), vertexEntry, nullptr);
				pIndexBuffer->Allocate<uInt16>(renderable.modelData.indices.Size(), indexEntry, nullptr);
			}
			else
			{
				float*	pVertexData;
				uInt16* pIndexData;

				pVertexBuffer->Map();
				pIndexBuffer->Map();

				pVertexBuffer->Allocate<float>(renderable.modelData.vertices.Size(), vertexEntry, &pVertexData);
				pIndexBuffer->Allocate<uInt16>(renderable.modelData.indices.Size(), indexEntry, &pIndexData);

				CopyMeshData(renderable.modelData, pVertexData, pIndexData);
			}

			bufferLocation.vertexEntry			= vertexEntry;
			bufferLocation.indexEntry			= indexEntry;
			bufferLocation.vertexBufferIndex	= mVertexBuffers.Size() - 1;
			bufferLocation.indexBufferIndex		= mIndexBuffers.Size() - 1;
		}

		mMeshBufferLookup.Put(renderable.modelURIHash, bufferLocation);

		outFound = false;

		return bufferLocation;
	}

	MeshBufferLocation VulkanRenderScene::GetOrAllocateMeshStagingBuffers(MeshComponent& renderable, bool& outFound)
	{
		auto& it = mMeshStagingBufferLookup.Find(renderable.modelURIHash);

		if (it != mMeshStagingBufferLookup.End())
		{
			outFound = true;
			return it->value;
		}

		MeshBufferLocation		bufferLocation = {};
		VulkanMultiBufferEntry	vertexStagingEntry;
		VulkanMultiBufferEntry	indexStagingEntry;

		float*	pVertexStagingData;
		uInt16* pIndexStagingData;

		if (!mSettings.useUniqueMeshStagingBuffers)
		{
			// Temporary, may be more buffers
			constexpr const uSize vertexStagingIndex = 0;
			constexpr const uSize indexStagingIndex = 0;

			VulkanMultiBuffer* pVertexStagingBuffer = &mVertexStagingBuffers[vertexStagingIndex];
			VulkanMultiBuffer* pIndexStagingBuffer = &mIndexStagingBuffers[indexStagingIndex];

			pVertexStagingBuffer->Map();
			pIndexStagingBuffer->Map();

			pVertexStagingBuffer->Allocate<float>(renderable.modelData.vertices.Size(), vertexStagingEntry, &pVertexStagingData);
			pIndexStagingBuffer->Allocate<uInt16>(renderable.modelData.indices.Size(), indexStagingEntry, &pIndexStagingData);

			CopyMeshData(renderable.modelData, pVertexStagingData, pIndexStagingData);

			bufferLocation.vertexEntry			= vertexStagingEntry;
			bufferLocation.indexEntry			= indexStagingEntry;
			bufferLocation.vertexBufferIndex	= vertexStagingIndex;
			bufferLocation.indexBufferIndex		= indexStagingIndex;
		}
		else
		{
			VulkanBufferInfo stagingVertexBufferInfo = {};
			stagingVertexBufferInfo.sizeBytes			= renderable.modelData.vertices.Size() * sizeof(float);
			stagingVertexBufferInfo.vkBufferUsage		= VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
			stagingVertexBufferInfo.vkMemoryProperties	= VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

			VulkanMultiBuffer* pVertexStagingBuffer = 
				&mVertexStagingBuffers.PushBack(VulkanMultiBuffer(mpResourceManager->CreateBuffer(mpDevice, stagingVertexBufferInfo)));

			VulkanBufferInfo stagingIndexBufferInfo = {};
			stagingIndexBufferInfo.sizeBytes			= renderable.modelData.indices.Size() * sizeof(uInt16);
			stagingIndexBufferInfo.vkBufferUsage		= VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
			stagingIndexBufferInfo.vkMemoryProperties	= VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

			VulkanMultiBuffer* pIndexStagingBuffer = 
				&mIndexStagingBuffers.PushBack(VulkanMultiBuffer(mpResourceManager->CreateBuffer(mpDevice, stagingIndexBufferInfo)));

			pVertexStagingBuffer->Map();
			pIndexStagingBuffer->Map();

			pVertexStagingBuffer->Allocate<float>(renderable.modelData.vertices.Size(), vertexStagingEntry, &pVertexStagingData);
			pIndexStagingBuffer->Allocate<uInt16>(renderable.modelData.indices.Size(), indexStagingEntry, &pIndexStagingData);

			CopyMeshData(renderable.modelData, pVertexStagingData, pIndexStagingData);

			bufferLocation.vertexEntry			= vertexStagingEntry;
			bufferLocation.indexEntry			= indexStagingEntry;
			bufferLocation.vertexBufferIndex	= mVertexStagingBuffers.Size() - 1;
			bufferLocation.indexBufferIndex		= mIndexStagingBuffers.Size() - 1;
		}

		mMeshStagingBufferLookup.Put(renderable.modelURIHash, bufferLocation);

		outFound = false;

		return bufferLocation;
	}

	PerModelBufferLocation VulkanRenderScene::AllocatePerModelBuffer(TransformComponent& transform)
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

			pPerModelBuffer->Allocate<uInt8>(sizeof(Mat4f) * 2, perModelEntry, nullptr); // <--- TEMP SIZE

			bufferLocation.perModelEntry		= perModelEntry;
			bufferLocation.perModelBufferIndex	= perModelIndex;
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
			perModelBufferInfo.sizeBytes			= sizeof(Mat4f) * 2; // <--- TEMP SIZE
			perModelBufferInfo.vkBufferUsage		= VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | usageFlags;
			perModelBufferInfo.vkMemoryProperties	= memoryFlags;

			pPerModelBuffer = &mPerModelBuffers.PushBack(VulkanMultiBuffer(mpResourceManager->CreateBuffer(mpDevice, perModelBufferInfo)));

			if (mSettings.useUniformStaging)
			{
				pPerModelBuffer->Allocate<uInt8>(sizeof(Mat4f) * 2, perModelEntry, nullptr); // <--- TEMP SIZE
			}
			else
			{
				uInt8* pPerModelData;

				pPerModelBuffer->Map();
				pPerModelBuffer->Allocate<uInt8>(sizeof(Mat4f) * 2, perModelEntry, &pPerModelData); // <--- TEMP SIZE

				///// TEMP
				*(Mat4f*)(pPerModelData) = transform.GetMatrix();
				*(Mat4f*)(pPerModelData + sizeof(Mat4f)) = Mat4f().SetTranslation({ 0.0f, 0.0f, -1.0f }) * Mat4f().SetPerspective(ToRadians(90.0f),
					(float)640 / (float)480, 0.001f, 1000.0f);
				/////
			}

			bufferLocation.perModelEntry		= perModelEntry;
			bufferLocation.perModelBufferIndex	= mPerModelBuffers.Size() - 1;
		}

		return bufferLocation;
	}

	PerModelBufferLocation VulkanRenderScene::AllocatePerModelStagingBuffer(TransformComponent& transform)
	{
		PerModelBufferLocation	bufferLocation = {};
		VulkanMultiBufferEntry	perModelStagingEntry;
		uInt8*					pPerModelStagingData;

		if (!mSettings.useUniqueUniformStagingBuffers)
		{
			// Temporary, may be more buffers
			constexpr const uSize perModelStagingIndex = 0;

			VulkanMultiBuffer* pPerModelBuffer = &mPerModelStagingBuffers[perModelStagingIndex];

			pPerModelBuffer->Map();
			pPerModelBuffer->Allocate<uInt8>(sizeof(Mat4f) * 2, perModelStagingEntry, &pPerModelStagingData); // <--- TEMP SIZE

			///// TEMP
			//Mat4f transformData = transform.GetMatrix();
			//CopyPerModelData((uInt8*)&transformData, pPerModelStagingData, sizeof(Mat4f));
			*(Mat4f*)(pPerModelStagingData) = transform.GetMatrix();
			*(Mat4f*)(pPerModelStagingData + sizeof(Mat4f)) = Mat4f().SetTranslation({ 0.0f, 0.0f, -1.0f }) * Mat4f().SetPerspective(ToRadians(90.0f),
				(float)640 / (float)480, 0.001f, 1000.0f);
			/////

			bufferLocation.perModelEntry		= perModelStagingEntry;
			bufferLocation.perModelBufferIndex	= perModelStagingIndex;
		}
		else
		{
			VulkanBufferInfo perModelStagingBufferInfo = {};
			perModelStagingBufferInfo.sizeBytes				= sizeof(Mat4f) * 2; // <--- TEMP SIZE
			perModelStagingBufferInfo.vkBufferUsage			= VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
			perModelStagingBufferInfo.vkMemoryProperties	= VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

			VulkanMultiBuffer* pPerModelBuffer = 
				&mPerModelStagingBuffers.PushBack(VulkanMultiBuffer(mpResourceManager->CreateBuffer(mpDevice, perModelStagingBufferInfo)));

			pPerModelBuffer->Map();
			pPerModelBuffer->Allocate<uInt8>(sizeof(Mat4f) * 2, perModelStagingEntry, &pPerModelStagingData); // <--- TEMP SIZE

			///// TEMP
			*(Mat4f*)(pPerModelStagingData) = transform.GetMatrix();
			*(Mat4f*)(pPerModelStagingData + sizeof(Mat4f)) = Mat4f().SetTranslation({ 0.0f, 0.0f, -1.0f }) * Mat4f().SetPerspective(ToRadians(90.0f),
				(float)640 / (float)480, 0.001f, 1000.0f);
			/////

			bufferLocation.perModelEntry		= perModelStagingEntry;
			bufferLocation.perModelBufferIndex	= mPerModelStagingBuffers.Size() - 1;
		}

		return bufferLocation;
	}

	void VulkanRenderScene::BuildScene(EntityWorld* pWorld)
	{
		auto& renderableView = pWorld->CreateView<MeshComponent, TransformComponent>();

		for (Entity& entity : renderableView)
		{
			MeshComponent& renderableComponent = pWorld->Get<MeshComponent>(entity);
			TransformComponent& transformComponent = pWorld->Get<TransformComponent>(entity);

			bool meshBufferFound;

			MeshBufferLocation meshBufferLocation = GetOrAllocateMeshBuffers(renderableComponent, meshBufferFound);
			PerModelBufferLocation perModelBufferLocation = AllocatePerModelBuffer(transformComponent);

			if (mSettings.useMeshStaging)
			{
				bool meshStagingBufferFound;

				MeshBufferLocation meshStagingBufferLocation = GetOrAllocateMeshStagingBuffers(renderableComponent, meshStagingBufferFound);

				if (!meshBufferFound)
				{
					TransferCommand vertexTransfer = {};
					vertexTransfer.pSrcBuffer	= &mVertexStagingBuffers[meshStagingBufferLocation.vertexBufferIndex];
					vertexTransfer.pDestBuffer	= &mVertexBuffers[meshBufferLocation.vertexBufferIndex];
					vertexTransfer.srcEntry		= meshStagingBufferLocation.vertexEntry;
					vertexTransfer.destEntry	= meshBufferLocation.vertexEntry;

					mReadyTransfers.PushBack(vertexTransfer);

					TransferCommand indexTransfer = {};
					indexTransfer.pSrcBuffer	= &mIndexStagingBuffers[meshStagingBufferLocation.indexBufferIndex];
					indexTransfer.pDestBuffer	= &mIndexBuffers[meshBufferLocation.indexBufferIndex];
					indexTransfer.srcEntry		= meshStagingBufferLocation.indexEntry;
					indexTransfer.destEntry		= meshBufferLocation.indexEntry;

					mReadyTransfers.PushBack(indexTransfer);
				}
			}

			// @TODO: This is using a copy command for each object when one is sufficient for ubos

			if (mSettings.useUniformStaging && !mSettings.usePerModelPushConstants)
			{
				PerModelBufferLocation perModelBufferStagingLocation = AllocatePerModelStagingBuffer(transformComponent);

				TransferCommand perModelTransfer = {};
				perModelTransfer.pSrcBuffer		= &mPerModelStagingBuffers[perModelBufferStagingLocation.perModelBufferIndex];
				perModelTransfer.pDestBuffer	= &mPerModelBuffers[perModelBufferLocation.perModelBufferIndex];
				perModelTransfer.srcEntry		= perModelBufferStagingLocation.perModelEntry;
				perModelTransfer.destEntry		= perModelBufferLocation.perModelEntry;

				mReadyTransfers.PushBack(perModelTransfer);
			}

			VulkanRenderable renderable = {};
			renderable.meshLocation		= meshBufferLocation;
			renderable.perModelLocation	= perModelBufferLocation;
			renderable.indexCount		= renderableComponent.modelData.indices.Size();
			renderable.materialId		= 0;

			mRenderables.PushBack(renderable);
		}

		// Sort
	}

	void VulkanRenderScene::RecordTransfers(VulkanCommandRecorder* pRecorder)
	{
		while (!mReadyTransfers.IsEmpty())
		{
			TransferCommand transfer = mReadyTransfers.PopFront();

			pRecorder->CopyBuffer(
				transfer.pSrcBuffer->GetVulkanBuffer(), transfer.pDestBuffer->GetVulkanBuffer(), 
				transfer.srcEntry.sizeBytes, transfer.srcEntry.offset, transfer.destEntry.offset);
		}
	}

	void VulkanRenderScene::RecordRender(VulkanCommandRecorder* pRecorder, VulkanGraphicsPipeline* pPipeline)
	{
		if (true)//(!mSettings.useUniqueMeshBuffers)
		{
			if (mSettings.useDrawIndirect)
			{

			}
			else
			{
				
				for (VulkanRenderable& renderable : mRenderables)
				{
					pRecorder->SetIndexBuffer(mIndexBuffers[renderable.meshLocation.indexBufferIndex].GetVulkanBuffer(), 
						renderable.meshLocation.indexEntry.offset, VK_INDEX_TYPE_UINT16);

					pRecorder->SetVertexBuffers({ { mVertexBuffers[renderable.meshLocation.vertexBufferIndex].GetVulkanBuffer() , 
						renderable.meshLocation.vertexEntry.offset } });

					VulkanUniformBinding binding = {};
					binding.binding = 0;
					binding.pBuffer = mPerModelBuffers[renderable.perModelLocation.perModelBufferIndex].GetVulkanBuffer();
					binding.offset	= renderable.perModelLocation.perModelEntry.offset;
					binding.range	= renderable.perModelLocation.perModelEntry.sizeBytes;

					pRecorder->BindUniforms(pPipeline, 0, { binding });
					pRecorder->DrawIndexed(1, renderable.indexCount, 0/*renderable.meshLocation.indexEntry.offset / sizeof(uInt16)*/);
				}
			}
		}
		else
		{

		}

	}
}