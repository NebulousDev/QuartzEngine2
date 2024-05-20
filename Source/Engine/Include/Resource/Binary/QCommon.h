#pragma once

#include "Types/Types.h"

namespace Quartz
{
	enum QStringEncoding : uInt16
	{
		Q_STRING_UTF8	= 1,
		Q_STRING_UTF16	= 2,
		Q_STRING_UTF32	= 4
	};

	using QStringID = uInt32;

	struct QStringTable							// 256 bits
	{
		uInt32			stringCount;			// 32 bits
		QStringEncoding	encoding;				// 16 bits
		uInt16			_reserved0;				// 16 bits
		uInt64			strsOffset;				// 64 bits
		uInt64			strsSizeBytes;			// 64 bits
		uInt64			nextExtOffset;			// 64 bits
	};
}