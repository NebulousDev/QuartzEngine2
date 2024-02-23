#include "Component/CameraComponent.h"

namespace Quartz
{
	CameraComponent::CameraComponent() :
		fov(70.0f), near(0.001f), far(1000.0f)
	{
		// Nothing
	}

	CameraComponent::CameraComponent(float fovDeg, float nearPlane, float farPlane) :
		fov(fovDeg), near(nearPlane), far(farPlane)
	{
		// Nothing
	}

	Mat4f CameraComponent::GetProjectionMatrix(float width, float hight)
	{
		return Mat4f().SetPerspective(ToRadians(fov), width / hight, near, far);
	}
}
