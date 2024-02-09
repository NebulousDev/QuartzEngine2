#include "Vulkan/VulkanMultiBuffer.h"

#include "Log.h"

namespace Quartz
{
	VulkanMultiBuffer::VulkanMultiBuffer(VulkanBuffer* pBuffer) :
		mpBuffer(pBuffer), mpMappedData(nullptr), mUsedBytes(0)
	{
		mBufferWriter = VulkanBufferWriter(pBuffer);

		VulkanMultiBufferEntry* pInitialEntry = new VulkanMultiBufferEntry();
		pInitialEntry->offset		= 0;
		pInitialEntry->sizeBytes	= pBuffer->sizeBytes;
		pInitialEntry->allocated	= false;
		pInitialEntry->pNext		= nullptr;
		pInitialEntry->pLast		= nullptr;

		mpHead = pInitialEntry;
		mpTail = pInitialEntry;
		mpFirstEmpty = pInitialEntry;
	}

	void* VulkanMultiBuffer::AllocateBytes(uSize sizeBytes, VulkanMultiBufferEntry& outEntry)
	{
		if (!mpMappedData)
		{
			return nullptr;
		}

		if (mUsedBytes + sizeBytes > mpBuffer->sizeBytes)
		{
			LogError("Error writing to MultiBuffer: Out of buffer memory.");
			return nullptr;
		}

		uSize emptySize = mpFirstEmpty->sizeBytes - sizeBytes;
		VulkanMultiBufferEntry* pNextEmpty = mpFirstEmpty;

		if (emptySize < 0)
		{
			while (pNextEmpty->pNext != nullptr && pNextEmpty->sizeBytes > sizeBytes)
			{
				pNextEmpty = pNextEmpty->pNext;
			}

			if (pNextEmpty == nullptr)
			{
				LogError("Error writing to MultiBuffer: Out of buffer memory.");
				return nullptr;
			}
		}

		emptySize = pNextEmpty->sizeBytes - sizeBytes;

		VulkanMultiBufferEntry* pEntry = new VulkanMultiBufferEntry();
		pEntry->offset		= pNextEmpty->offset;
		pEntry->sizeBytes	= sizeBytes;
		pEntry->allocated	= true;
		pEntry->pLast		= pNextEmpty->pLast;
		pEntry->pNext		= pNextEmpty;

		if (pEntry->pLast)
		{
			pEntry->pLast->pNext = pEntry;
		}

		if (pEntry->pNext)
		{
			pEntry->pNext->pLast = pEntry;
		}
			
		if (emptySize == 0)
		{
			pEntry->pNext = pNextEmpty->pNext;

			VulkanMultiBufferEntry* pOldEmpty = pNextEmpty;

			while (pNextEmpty->pNext != nullptr && pNextEmpty->sizeBytes > sizeBytes)
			{
				pNextEmpty = pNextEmpty->pNext;
			}

			if (mpFirstEmpty == pNextEmpty)
			{
				mpFirstEmpty = nullptr;
				pNextEmpty = nullptr;
			}
			else
			{
				mpFirstEmpty = pNextEmpty;
			}

			delete pOldEmpty;
		}
		else
		{
			mpFirstEmpty->pLast		= pEntry;
			mpFirstEmpty->offset	= mpFirstEmpty->offset + sizeBytes;
			mpFirstEmpty->sizeBytes = emptySize;
		}

		if (pEntry->pLast == nullptr)
		{
			mpHead = pEntry;
		}

		if (pEntry->pNext == nullptr)
		{
			mpTail = pEntry;
		}

		mUsedBytes += sizeBytes;

		outEntry = *pEntry;

		return mpMappedData + pEntry->offset;
	}

	void VulkanMultiBuffer::Free(const VulkanMultiBufferEntry& entry)
	{
		VulkanMultiBufferEntry* pEntry = mpHead;
		VulkanMultiBufferEntry* pFirstFree = nullptr;

		while (pEntry->pNext != nullptr)
		{
			if (!pFirstFree && pEntry->allocated == false)
			{
				pFirstFree = pEntry;
			}

			if (pEntry->offset == entry.offset)
			{
				pEntry->allocated = false;

				if (pEntry->pNext && !pEntry->pNext->allocated)
				{
					VulkanMultiBufferEntry* pNextOld = pEntry->pNext;

					if (pEntry->pNext->pNext)
					{
						pEntry->pNext->pNext->pLast = pEntry;
					}

					pEntry->pNext = pNextOld->pNext;
					pEntry->sizeBytes += pNextOld->sizeBytes;

					delete pNextOld;
				}

				if (pEntry->pLast && !pEntry->pLast->allocated)
				{
					VulkanMultiBufferEntry* pLastOld = pEntry->pLast;

					if (pEntry->pLast->pLast)
					{
						pEntry->pLast->pLast->pNext = pEntry;
					}

					pEntry->pLast = pLastOld->pLast;
					pEntry->sizeBytes += pLastOld->sizeBytes;
					pEntry->offset = pLastOld->offset;

					delete pLastOld;
				}

				if (!pFirstFree || pEntry->pLast == pFirstFree)
				{
					mpFirstEmpty = pEntry;
				}

				mUsedBytes -= pEntry->sizeBytes;

				return;
			}

			pEntry = pEntry->pNext;
		}

		if (pEntry == nullptr)
		{
			LogError("Error freeing VulkanMultiBuffer entry: Entry not found.");
		}
	}

	bool VulkanMultiBuffer::Map()
	{
		mpMappedData = mBufferWriter.Map<uInt8>();

		return mpMappedData != nullptr;
	}

	bool VulkanMultiBuffer::Unmap()
	{
		mBufferWriter.Unmap();
		mpMappedData = nullptr;

		return true;
	}
}

