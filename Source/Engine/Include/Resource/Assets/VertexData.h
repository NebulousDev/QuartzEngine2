#pragma once

#include "Types/Types.h"
#include "Types/Array.h"
#include "Types/Special/ByteBuffer.h"

namespace Quartz
{
#define VERTEX_FLOAT_SIZE			4
#define VERTEX_INT_SIZE				4
#define VERTEX_UINT_SIZE			4
#define VERTEX_PACKED_INT_3_SIZE	4
#define VERTEX_PACKED_UINT_3_SIZE	4
#define VERTEX_PACKED_FLOAT_3_SIZE	4

#define INDEX_UINT8_SIZE			1
#define INDEX_UINT16_SIZE			2
#define INDEX_UINT32_SIZE			4

#define INDEX_MAX_UINT8				256
#define INDEX_MAX_UINT16			65536
#define INDEX_MAX_UINT32			4294967296

	enum VertexAttribute : uInt32
	{
		VERTEX_ATTRIBUTE_INVALID = 0,
		VERTEX_ATTRIBUTE_POSITION,
		VERTEX_ATTRIBUTE_NORMAL,
		VERTEX_ATTRIBUTE_BINORMAL,
		VERTEX_ATTRIBUTE_TANGENT,
		VERTEX_ATTRIBUTE_TEXCOORD,
		VERTEX_ATTRIBUTE_COLOR,
		VERTEX_ATTRIBUTE_DATA
	};

	enum VertexFormat : uInt32
	{
		VERTEX_FORMAT_INVALID			= 0,
		VERTEX_FORMAT_FLOAT				= ( 1 << 16) | VERTEX_FLOAT_SIZE * 1,
		VERTEX_FORMAT_FLOAT2			= ( 2 << 16) | VERTEX_FLOAT_SIZE * 2,
		VERTEX_FORMAT_FLOAT3			= ( 3 << 16) | VERTEX_FLOAT_SIZE * 3,
		VERTEX_FORMAT_FLOAT4			= ( 4 << 16) | VERTEX_FLOAT_SIZE * 4,
		VERTEX_FORMAT_INT				= ( 5 << 16) | VERTEX_INT_SIZE * 1,
		VERTEX_FORMAT_INT2				= ( 6 << 16) | VERTEX_INT_SIZE * 2,
		VERTEX_FORMAT_INT3				= ( 7 << 16) | VERTEX_INT_SIZE * 3,
		VERTEX_FORMAT_INT4				= ( 8 << 16) | VERTEX_INT_SIZE * 4,
		VERTEX_FORMAT_UINT				= ( 9 << 16) | VERTEX_UINT_SIZE * 1,
		VERTEX_FORMAT_UINT2				= (10 << 16) | VERTEX_UINT_SIZE * 2,
		VERTEX_FORMAT_UINT3				= (11 << 16) | VERTEX_UINT_SIZE * 3,
		VERTEX_FORMAT_UINT4				= (12 << 16) | VERTEX_UINT_SIZE * 4,
		VERTEX_FORMAT_INT_2_10_10_10	= (13 << 16) | VERTEX_PACKED_INT_3_SIZE,
		VERTEX_FORMAT_UINT_2_10_10_10	= (14 << 16) | VERTEX_PACKED_UINT_3_SIZE,
		VERTEX_FORMAT_FLOAT_10_11_11	= (15 << 16) | VERTEX_PACKED_FLOAT_3_SIZE
	};

	enum IndexFormat : uInt32
	{
		INDEX_FORMAT_INVALID	= 0,
		INDEX_FORMAT_UINT8		= (1 << 16) | INDEX_UINT8_SIZE,
		INDEX_FORMAT_UINT16		= (2 << 16) | INDEX_UINT16_SIZE,
		INDEX_FORMAT_UINT32		= (3 << 16) | INDEX_UINT32_SIZE
	};

	struct VertexElement
	{
		VertexAttribute	attribute;
		VertexFormat	format;

		inline uSize FormatSize() const
		{
			return (uSize)(format & 0x0000FFFF);
		}

		inline uSize FormatID() const
		{
			return (uSize)((format & 0xFFFF0000) >> 16);
		}
	};

	struct IndexElement
	{
		IndexFormat format;

		inline uSize FormatSize() const
		{
			return (uSize)(format & 0x0000FFFF);
		}

		inline uSize FormatID() const
		{
			return (uSize)((format & 0xFFFF0000) >> 16);
		}
	};

	struct VertexData
	{
		Array<VertexElement, 8>	elements;
		IndexElement			index;
		ByteBuffer*				pVertexBuffer;
		ByteBuffer*				pIndexBuffer;
	};
}