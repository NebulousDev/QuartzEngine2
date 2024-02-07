#pragma once

#include "Types/Types.h"
#include "VulkanDevice.h"

namespace Quartz
{
	struct VulkanBufferInfo
	{
		uSize					sizeBytes;
		VkBufferUsageFlags		vkBufferUsage;
		VkMemoryPropertyFlags	vkMemoryProperties;
	};

	struct VulkanBuffer
	{
		VkBuffer				vkBuffer;
		VulkanDevice*			pDevice;
		uSize					sizeBytes;
		VkDeviceMemory			vkMemory;
		VkBufferUsageFlags		vkUsage;
		VkMemoryPropertyFlags	vkMemoryProperties;
	};

	class VulkanBufferWriter
	{
	private:
		VulkanBuffer*	mpBuffer;
		bool			mMapped;
		void*			mpMappedData;
		uSize			mMappedSize;
		uSize			mMappedOffset;

	private:
		void* MapBytes(uSize size, uSize offset);

	public:
		VulkanBufferWriter(VulkanBuffer* pBuffer);

		template<typename Data>
		Data* Map()
		{
			return static_cast<Data*>(MapBytes(mpBuffer->sizeBytes, 0));
		}

		template<typename Data>
		Data* Map(uSize count, uSize offset)
		{
			return static_cast<Data*>(MapBytes(count * sizeof(Data), offset * sizeof(Data)));
		}

		void Unmap();
	};
}