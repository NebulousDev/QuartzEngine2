#pragma once

#include "Resource/Asset.h"
#include "VertexData.h"
#include "Types/Array.h"

namespace Quartz
{
	struct Mesh
	{
		String					name;
		uInt32					lod;
		uInt32					materialIdx;
		Array<VertexElement, 8>	elements;
		IndexElement			indexElement;
		uInt64					verticesStartBytes;
		uInt64					verticesSizeBytes;
		uInt32					indicesStartBytes;
		uInt32					indicesSizeBytes;
	};

	struct Model : public Asset
	{
		VertexData	vertexData;
		Array<Mesh>	meshes;

		inline Model() = default;
		inline Model(File* pSourceFile) : Asset(pSourceFile) {};

		inline String GetAssetTypeName() const override { return "Model"; }
	}; 
}