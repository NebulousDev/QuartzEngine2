#pragma once

#include "EngineAPI.h"
#include "../ModelData.h"
#include "Entity/Component.h"

namespace Quartz
{
	struct TerrainSettings
	{
		uSize			resolution;
		uInt64			seed;
		float			scale;
		float			lacunarity;
		Array<float>	octaveWeights;
	};

	struct QUARTZ_ENGINE_API TerrainComponent : public Component<TerrainComponent>
	{
		TerrainSettings settings;

		TerrainComponent();
		TerrainComponent(const TerrainSettings& settings);
	};
}