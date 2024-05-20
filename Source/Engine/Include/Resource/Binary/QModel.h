#pragma once

#include "QCommon.h"

namespace Quartz
{
	enum QModelVertexAttribute : uInt8
	{
		QMODEL_VERTEX_ATTRIBUTE_INVALID = 0,
		QMODEL_VERTEX_ATTRIBUTE_POSITION,
		QMODEL_VERTEX_ATTRIBUTE_NORMAL,
		QMODEL_VERTEX_ATTRIBUTE_BINORMAL,
		QMODEL_VERTEX_ATTRIBUTE_TANGENT,
		QMODEL_VERTEX_ATTRIBUTE_TEXCOORD,
		QMODEL_VERTEX_ATTRIBUTE_COLOR,
		QMODEL_VERTEX_ATTRIBUTE_DATA
	};

	enum QModelVertexFormat : uInt8
	{
		QMODEL_VERTEX_FORMAT_INVALID = 0,
		QMODEL_VERTEX_FORMAT_FLOAT,
		QMODEL_VERTEX_FORMAT_FLOAT2,
		QMODEL_VERTEX_FORMAT_FLOAT3,
		QMODEL_VERTEX_FORMAT_FLOAT4,
		QMODEL_VERTEX_FORMAT_INT,
		QMODEL_VERTEX_FORMAT_INT2,
		QMODEL_VERTEX_FORMAT_INT3,
		QMODEL_VERTEX_FORMAT_INT4,
		QMODEL_VERTEX_FORMAT_UINT,
		QMODEL_VERTEX_FORMAT_UINT2,
		QMODEL_VERTEX_FORMAT_UINT3,
		QMODEL_VERTEX_FORMAT_UINT4,
		QMODEL_VERTEX_FORMAT_INT_2_10_10_10,
		QMODEL_VERTEX_FORMAT_UINT_2_10_10_10,
		QMODEL_VERTEX_FORMAT_FLOAT_10_11_11
	};

	enum QModelIndexFormat : uInt8
	{
		QMODEL_INDEX_FORMAT_INVALID = 0,
		QMODEL_INDEX_FORMAT_UINT8,
		QMODEL_INDEX_FORMAT_UINT16,
		QMODEL_INDEX_FORMAT_UINT32
	};

#pragma pack(push,1)

	struct QModelMeshElement						// 32 bits
	{
		QModelVertexAttribute	attribute;			// 8 bits
		QModelVertexFormat		format;				// 8 bits
		uInt16					_reserved0;			// 16 bits
	};

	struct QModelMesh								// 448 bits
	{
		QStringID			nameID;					// 32 bitss
		QStringID			materialPathID;			// 32 bits
		uInt16				lodIdx;					// 16 bits
		uInt8				_reserved0;				// 8 bits
		QModelIndexFormat	indexFormat;			// 8 bits
		uInt16				elementCount;			// 16 bits
		uInt16				_reserved1;				// 16 bits
		uInt64				elementOffset;			// 64 bits
		uInt64				verticesOffset;			// 64 bits
		uInt64				verticesSizeBytes;		// 64 bits
		uInt64				indicesOffset;			// 64 bits
		uInt64				indicesSizeBytes;		// 64 bits
	};

	struct QModelMeshTable							// 384 bits
	{
		uInt32	meshCount;							// 32 bits
		uInt16	_reserved0;							// 32 bits
		uInt64	meshesOffset;						// 64 bits
		uInt64	meshesSizeBytes;					// 64 bits
		uInt64	vertexBufferSizeBytes;				// 64 bits
		uInt64	indexBufferSizeBytes;				// 64 bits
		uInt64	nextExtOffset;						// 64 bits
	};

	struct QModel									// -- bits
	{
		char			magic[8] = "QModel";		// 32 bits
		uInt16			versionMajor;				// 16 bits
		uInt16			versionMinor;				// 16 bits
		uInt64			_reserved0;					// 64 bits
		QStringTable	stringTable;				// 256 bits
		QModelMeshTable	meshTable;					// 384 bits
		uInt64			nextExtOffset;				// 64 bits
	};

#pragma pack(pop)

}