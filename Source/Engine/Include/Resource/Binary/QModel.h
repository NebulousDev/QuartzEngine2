#pragma once

#include "QCommon.h"

namespace Quartz
{
	enum QModelVertexAttribute : uInt8
	{
		QMODEL_VERTEX_ATTRIBUTE_INVALID = 0,
		QMODEL_VERTEX_ATTRIBUTE_POSITION,
		QMODEL_VERTEX_ATTRIBUTE_NORMAL,
		QMODEL_VERTEX_ATTRIBUTE_BITANGENT,
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
		QMODEL_VERTEX_FORMAT_FLOAT_10_11_11,
		QMODEL_VERTEX_FORMAT_FLOAT_16_16_16_16
	};

	enum QModelIndexFormat : uInt8
	{
		QMODEL_INDEX_FORMAT_INVALID = 0,
		QMODEL_INDEX_FORMAT_UINT8,
		QMODEL_INDEX_FORMAT_UINT16,
		QMODEL_INDEX_FORMAT_UINT32
	};

#pragma pack(push,1)

	struct QModelVertexElement						// 64 bits
	{
		QModelVertexAttribute	attribute;			// 8 bits
		QModelVertexFormat		format;				// 8 bits
		uInt16					_reserved0;			// 16 bits
		uInt16					offsetBytes;		// 16 bits
		uInt16					sizeBytes;			// 16 bits
	};

	struct QModelIndexElement						// 64 bits
	{
		QModelIndexFormat	format;					// 8 bits
		uInt8				_reserved0;				// 8 bits
		uInt16				sizeBytes;				// 16 bits
		uInt32				_reserved1;				// 32 bits
	};

	struct QModelVertexElementTable					// 192 bits
	{
		uInt32 elementCount;						// 32 bits
		uInt32 _reserved0;							// 32 bits
		uInt64 elementsOffset;						// 64 bits
		uInt64 elementsSizeBytes;					// 64 bits
	};

	struct QModelVertexStream						// 384 bits
	{
		uInt32					 streamIdx;			// 32 bits
		uInt16					 _reserved0;		// 16 bits
		uInt16					 strideBytes;		// 16 bits
		QModelVertexElementTable elementTable;		// 192 bits
		uInt64					 streamOffset;		// 64 bits
		uInt64					 streamSizeBytes;	// 64 bits
	};

	struct QModelIndexStream						// 256 bits
	{
		QModelIndexElement	indexElement;			// 64 bits
		uInt32				indexCount;				// 32 bits
		uInt32				maxIndex;				// 32 bits
		uInt64				streamOffset;			// 64 bits
		uInt64				streamSizeBytes;		// 64 bits
	};

	struct QModelStreamTable						// 384 bits
	{
		uInt32 streamCount;							// 32 bits
		uInt32 _reserved0;							// 32 bits
		uInt64 vertexStreamsOffset;					// 64 bits
		uInt64 vertexStreamsSizeBytes;				// 64 bits
		uInt64 indexStreamOffset;					// 64 bits
		uInt64 indexStreamSizeBytes;				// 64 bits
		uInt64 nextExtOffset;						// 64 bits
	};

	struct QModelMesh								// 192 bits
	{
		QStringID	nameID;							// 32 bitss
		uInt32		materialIdx;					// 32 bits
		uInt16		lodIdx;							// 16 bits
		uInt16		_reserved0;						// 16 bits
		uInt32		_reserved1;						// 32 bits
		uInt32		indexStart;						// 32 bits
		uInt32		indexCount;						// 32 bits
	};

	struct QModelMeshTable							// 256 bits
	{
		uInt32 meshCount;							// 32 bits
		uInt16 _reserved0;							// 32 bits
		uInt64 meshesOffset;						// 64 bits
		uInt64 meshesSizeBytes;						// 64 bits
		uInt64 nextExtOffset;						// 64 bits
	};

	struct QModel									// -- bits
	{
		char				magic[8] = "QModel";	// 32 bits
		uInt16				versionMajor;			// 16 bits
		uInt16				versionMinor;			// 16 bits
		uInt64				_reserved0;				// 64 bits
		QStringTable		stringTable;			// 256 bits
		QModelMeshTable		meshTable;				// 256 bits
		QModelStreamTable	streamTable;			// 384 bits
		uInt64				nextExtOffset;			// 64 bits
	};

#pragma pack(pop)

}