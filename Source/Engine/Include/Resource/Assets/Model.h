#pragma once

#include "Resource/Asset.h"
#include "Resource/Common.h"
#include "Types/Special/ByteBuffer.h"

namespace Quartz
{
	struct VertexElement
	{
		VertexAttribute	attribute;
		VertexFormat	format;
		uInt32			offsetBytes;
		uInt32			sizeBytes;
	};

	struct IndexElement
	{
		IndexFormat		format;
		uInt32			sizeBytes;
	};

	struct VertexStream
	{
		uInt32					streamIdx;
		Array<VertexElement, 8>	vertexElements;
		ByteBuffer*				pVertexBuffer;
		uInt32					strideBytes;
	};

	struct IndexStream
	{
		IndexElement	indexElement;
		ByteBuffer*		pIndexBuffer;
		uInt32			indexCount;
		uInt32			maxIndex;
	};

	struct Mesh
	{
		String name;
		uInt32 lod;
		uInt32 materialIdx;
		uInt32 indexStart;
		uInt32 indexCount;
	};

	struct Model : public Asset
	{
		Array<VertexStream, 8>	vertexStreams;
		IndexStream				indexStream;
		Array<Mesh>				meshes;

		inline Model() = default;
		inline Model(File* pSourceFile) : Asset(pSourceFile) {};

		inline String GetAssetTypeName() const override { return "Model"; }
	}; 
}