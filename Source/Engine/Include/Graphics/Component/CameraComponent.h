#pragma once

#include "EngineAPI.h"
#include "Entity/Component.h"

#include "Math/Math.h"

namespace Quartz
{
	struct QUARTZ_ENGINE_API CameraComponent : public Component<CameraComponent>
	{
		float fov;
		float near;
		float far;

		float width;
		float height;

		CameraComponent();
		CameraComponent(float width, float height, float fovDeg, float nearPlane, float farPlane);

		Mat4f GetProjectionMatrix();
	};
}