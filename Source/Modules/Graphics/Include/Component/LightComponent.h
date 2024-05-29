#pragma once

#include "GfxAPI.h"
#include "Entity/Component.h"
#include "Resource/Assets/Light.h"

namespace Quartz
{
	struct LightComponent : public Component<LightComponent>
	{
		PointLight pointLight;

		LightComponent() = default;
		inline LightComponent(const PointLight& pointLight) : pointLight(pointLight) {};
	};
}