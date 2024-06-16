#include "Graphics/Component/CameraComponent.h"

namespace Quartz
{
	CameraComponent::CameraComponent() :
		fov(70.0f), near(0.001f), far(1000.0f)
	{
		// Nothing
	}

	CameraComponent::CameraComponent(float width, float height, float fovDeg, float nearPlane, float farPlane) :
		fov(fovDeg), near(nearPlane), far(farPlane), width(width), height(height)
	{
		// Nothing
	}

	Mat4f CameraComponent::GetProjectionMatrix()
	{
		return Mat4f().SetPerspective(ToRadians(fov), width / height, near, far);
	}
}
