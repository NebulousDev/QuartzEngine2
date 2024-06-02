#pragma once

#include "Resource/Asset.h"
#include "Resource/Common.h"
#include "Types/Special/ByteBuffer.h"

namespace Quartz
{
	struct Image : public Asset
	{
		uInt32		width;
		uInt32		height;
		uInt32		depth;
		ImageFormat	format;
		ByteBuffer* pImageData;

		inline String GetAssetTypeName() const override { return "Image"; }
	};
}