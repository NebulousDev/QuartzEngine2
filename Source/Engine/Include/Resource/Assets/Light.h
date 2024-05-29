#pragma once

#include "Math/Math.h"

namespace Quartz
{
	struct PointLight
	{
		Vec3f color;
		float intensity;
	};

	struct DirectionalLight
	{
		Vec3f direction;
		Vec3f color;
		float intensity;
	};

	struct SpotLight
	{
		Vec3f direction;
		Vec3f color;
		float width;
		float intensity;
	};
}