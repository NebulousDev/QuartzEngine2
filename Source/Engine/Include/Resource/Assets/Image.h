#pragma once

#include "Resource/Asset.h"
#include "Types/Special/ByteBuffer.h"

namespace Quartz
{
	enum ImageFormat : uInt32
	{
		IMAGE_FORMAT_INVALID	= (0x000),
		IMAGE_FORMAT_R8			= (0x0001 << 16) | 1,
		IMAGE_FORMAT_R8G8		= (0x0002 << 16) | 2,
		IMAGE_FORMAT_R8G8B8		= (0x0004 << 16) | 3,
		IMAGE_FORMAT_R8G8B8A8	= (0x0008 << 16) | 4
	};

	struct Image : public Asset
	{
		uInt32		width;
		uInt32		height;
		uInt32		depth;
		ImageFormat	format;
		ByteBuffer* pImageData;

		inline uSize FormatSize() const
		{
			return (uSize)(format & 0x0000FFFF);
		}

		inline uSize FormatID() const
		{
			return (uSize)((format & 0xFFFF0000) >> 16);
		}
	};
}