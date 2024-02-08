#pragma once

#include "GfxDLL.h"
#include "VulkanBuffer.h"

namespace Quartz
{
	class QUARTZ_GRAPHICS_API VulkanBufferWriter
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