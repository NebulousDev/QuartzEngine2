#include "Vulkan/VulkanBufferWriter.h"
#include "Log.h"

#include <vulkan/vulkan.h>

namespace Quartz
{
	void* VulkanBufferWriter::MapBytes(uSize sizeBytes, uSize offset)
	{
		VulkanDevice* pDevice = mpBuffer->pDevice;

		// Mapping is the same, ignore
		if (mMappedSize == sizeBytes && mMappedOffset == mMappedOffset)
		{
			return mpMappedData;
		}

		// Check mapping bounds
		if (sizeBytes + offset > mpBuffer->sizeBytes)
		{
			LogError("Unable to map buffer memory: offset + size is out of bounds!");
			return nullptr;
		}

		// First unmap if previously mapped
		if (mMapped)
		{
			vkUnmapMemory(pDevice->vkDevice, mpBuffer->vkMemory);
		}

		if (vkMapMemory(pDevice->vkDevice, mpBuffer->vkMemory, offset, sizeBytes, 0, &mpMappedData) != VK_SUCCESS)
		{
			LogError("Unable to map buffer memory: vkMapMemory failed!");
			return nullptr;
		}

		mMapped = true;
		mMappedSize = sizeBytes;
		mMappedOffset = offset;

		return mpMappedData;
	}

	VulkanBufferWriter::VulkanBufferWriter() :
		mpBuffer(nullptr), mMapped(false), mpMappedData(nullptr), mMappedSize(0), mMappedOffset(0)
	{
		// Invalid state, use only for construction
	}

	VulkanBufferWriter::VulkanBufferWriter(VulkanBuffer* pBuffer) :
		mpBuffer(pBuffer), mMapped(false), mpMappedData(nullptr), mMappedSize(0), mMappedOffset(0)
	{
		if (!mpBuffer)
		{
			LogError("Error creating VulkanBufferWriter: pBuffer is null!");
		}
	}

	void* VulkanBufferWriter::Map()
	{
		return MapBytes(mpBuffer->sizeBytes, 0);
	}

	void VulkanBufferWriter::Unmap()
	{
		if (mMapped)
		{
			vkUnmapMemory(mpBuffer->pDevice->vkDevice, mpBuffer->vkMemory);

			mMapped = false;
			mpMappedData = nullptr;
			mMappedSize = 0;
			mMappedOffset = 0;
		}
	}
}