#pragma once

#include "EngineAPI.h"
#include "Entity/Component.h"
#include "Resource/Assets/Light.h"

namespace Quartz
{
	struct QUARTZ_ENGINE_API LightComponent : public Component<LightComponent>
	{
		PointLight pointLight;

		LightComponent() = default;
		inline LightComponent(const PointLight& pointLight) : pointLight(pointLight) {};
	};
}