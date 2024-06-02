#pragma once

#include "QCommon.h"

namespace Quartz
{
	struct QModelVertexElement						// 64 bits
	{
		VertexAttribute		attribute;				// 8 bits
		VertexFormat		format;					// 8 bits
		uInt16				_reserved0;				// 16 bits
		uInt16				offsetBytes;			// 16 bits
		uInt16				sizeBytes;				// 16 bits
	};

	struct QModelIndexElement						// 64 bits
	{
		IndexFormat			format;					// 8 bits
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