#pragma once

#include "Types/Types.h"
#include "Resource/Common.h"

namespace Quartz
{
	using QStringID = uInt32;

#pragma pack(push,1)

	struct QStringTable							// 256 bits
	{
		uInt32			stringCount;			// 32 bits
		StringEncoding	encoding;				// 16 bits
		uInt16			_reserved0;				// 16 bits
		uInt64			strsOffset;				// 64 bits
		uInt64			strsSizeBytes;			// 64 bits
		uInt64			nextExtOffset;			// 64 bits
	};

#pragma pack(pop)

}