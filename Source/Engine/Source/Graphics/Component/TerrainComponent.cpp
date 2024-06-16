#include "Graphics/Component/TerrainComponent.h"

namespace Quartz
{
	TerrainComponent::TerrainComponent() :
		settings()
	{
		settings.resolution = 200;
		settings.scale		= 550;
		settings.lacunarity = 2.0f;
		settings.octaveWeights = { 1.0f };
	}

	TerrainComponent::TerrainComponent(const TerrainSettings& settings) :
		settings(settings) { }
}

