#pragma once

#include "VulkanBufferWriter.h"
#include "Types/List.h"

namespace Quartz
{
	struct VulkanMultiBufferEntry
	{
		uInt32	offset;
		uInt32	sizeBytes;
		bool	allocated;

		VulkanMultiBufferEntry* pNext;
		VulkanMultiBufferEntry* pLast;
	};

	class VulkanMultiBuffer
	{
	private:
		VulkanBuffer*					mpBuffer;
		VulkanBufferWriter				mBufferWriter;
		VulkanMultiBufferEntry*			mpHead;
		VulkanMultiBufferEntry*			mpTail;
		VulkanMultiBufferEntry*			mpFirstEmpty;
		uInt8*							mpMappedData;
		uSize							mUsedBytes;

		void* AllocateBytes(uSize sizeBytes, VulkanMultiBufferEntry& outEntry);

	public:
		VulkanMultiBuffer(VulkanBuffer* pBuffer);
		
		template<typename Type>
		Type* Allocate(uSize count, VulkanMultiBufferEntry& outEntry)
		{
			return static_cast<Type*>(AllocateBytes(count * sizeof(Type), outEntry));
		}

		void Free(const VulkanMultiBufferEntry& entry);

		bool Map();
		bool Unmap();
	};
}