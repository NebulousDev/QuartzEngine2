#pragma once

#include "Types/Array.h"
#include "GfxAPI.h"
#include "ModelData.h"

namespace Quartz
{
	struct TerrainComponent
	{
		int data;
	};

	class QUARTZ_GRAPHICS_API TerrainRenderer
	{
	private:

	public:
		ModelData CreateTerrainChunkMesh(uSize resolution);
		Array<float> CreatePerlinNoiseTexture(uSize resolution, uInt64 seed, const Array<float>& octaveWeights);

	};
}