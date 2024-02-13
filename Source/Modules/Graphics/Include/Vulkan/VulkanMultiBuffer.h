#pragma once

#include "../GfxAPI.h"
#include "VulkanBufferWriter.h"

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

	class QUARTZ_GRAPHICS_API VulkanMultiBuffer
	{
	private:
		VulkanBuffer*					mpBuffer;
		VulkanBufferWriter				mBufferWriter;
		VulkanMultiBufferEntry*			mpHead;
		VulkanMultiBufferEntry*			mpTail;
		VulkanMultiBufferEntry*			mpFirstEmpty;
		uInt8*							mpMappedData;
		uSize							mUsedBytes;

		bool AllocateBytes(uSize sizeBytes, VulkanMultiBufferEntry& outEntry, void** ppOutData);

	public:
		VulkanMultiBuffer();
		VulkanMultiBuffer(VulkanBuffer* pBuffer);
		~VulkanMultiBuffer();
		
		// ppOutData will only return a value if the buffer is mapped. Pass nullptr to ignore.
		template<typename Type>
		bool Allocate(uSize count, VulkanMultiBufferEntry& outEntry, Type** ppOutData)
		{
			return AllocateBytes(count * sizeof(Type), outEntry, (void**)ppOutData);
		}

		void Free(const VulkanMultiBufferEntry& entry);
		void FreeAll();

		bool Map();
		bool Unmap();

		inline VulkanBuffer* GetVulkanBuffer() { return mpBuffer; }
		inline void* GetMappedData() { return mpMappedData; }
		inline uSize UsedBytes() { return mUsedBytes; }
		inline bool IsMapped() { return mpMappedData != nullptr; }
	};
}