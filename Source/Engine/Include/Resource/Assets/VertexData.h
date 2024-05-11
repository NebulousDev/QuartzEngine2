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
		VERTEX_ATTRIBUTE_INVALID,
		VERTEX_ATTRIBUTE_POSITION,
		VERTEX_ATTRIBUTE_NORMAL,
		VERTEX_ATTRIBUTE_BINORMAL,
		VERTEX_ATTRIBUTE_TANGENT,
		VERTEX_ATTRIBUTE_TEXCOORD,
		VERTEX_ATTRIBUTE_COLOR,
		VERTEX_ATTRIBUTE_DATA
	};

	enum VertexType : uInt32
	{
		VERTEX_TYPE_FLOAT			= (0x0000 << 16) | VERTEX_FLOAT_SIZE * 1,
		VERTEX_TYPE_FLOAT2			= (0x0001 << 16) | VERTEX_FLOAT_SIZE * 2,
		VERTEX_TYPE_FLOAT3			= (0x0002 << 16) | VERTEX_FLOAT_SIZE * 3,
		VERTEX_TYPE_FLOAT4			= (0x0004 << 16) | VERTEX_FLOAT_SIZE * 4,
		VERTEX_TYPE_INT				= (0x0008 << 16) | VERTEX_INT_SIZE * 1,
		VERTEX_TYPE_INT2			= (0x0010 << 16) | VERTEX_INT_SIZE * 2,
		VERTEX_TYPE_INT3			= (0x0020 << 16) | VERTEX_INT_SIZE * 3,
		VERTEX_TYPE_INT4			= (0x0040 << 16) | VERTEX_INT_SIZE * 4,
		VERTEX_TYPE_UINT			= (0x0080 << 16) | VERTEX_UINT_SIZE * 1,
		VERTEX_TYPE_UINT2			= (0x0100 << 16) | VERTEX_UINT_SIZE * 2,
		VERTEX_TYPE_UINT3			= (0x0200 << 16) | VERTEX_UINT_SIZE * 3,
		VERTEX_TYPE_UINT4			= (0x0400 << 16) | VERTEX_UINT_SIZE * 4,
		VERTEX_TYPE_INT_2_10_10_10	= (0x0800 << 16) | VERTEX_PACKED_INT_3_SIZE,
		VERTEX_TYPE_UINT_2_10_10_10	= (0x1000 << 16) | VERTEX_PACKED_UINT_3_SIZE,
		VERTEX_TYPE_FLOAT_10_11_11	= (0x2000 << 16) | VERTEX_PACKED_FLOAT_3_SIZE
	};

	enum IndexType : uInt32
	{
		INDEX_TYPE_UINT8	= (0x0000 << 16) | INDEX_UINT8_SIZE,
		INDEX_TYPE_UINT16	= (0x0001 << 16) | INDEX_UINT16_SIZE,
		INDEX_TYPE_UINT32	= (0x0002 << 16) | INDEX_UINT32_SIZE
	};

	struct VertexElement
	{
		VertexAttribute	attribute;
		VertexType		type;

		inline uSize Size() const
		{
			return (uSize)(type & 0x0000FFFF);
		}

		inline uSize ID() const
		{
			return (uSize)((type & 0xFFFF0000) >> 16);
		}
	};

	struct IndexElement
	{
		IndexType type;

		inline uSize Size() const
		{
			return (uSize)(type & 0x0000FFFF);
		}

		inline uSize ID() const
		{
			return (uSize)((type & 0xFFFF0000) >> 16);
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