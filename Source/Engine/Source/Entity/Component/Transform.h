#pragma once

#include "Entity/Component.h"
#include "Math/Math.h"

namespace Quartz
{
	struct TransformComponent : public Component<TransformComponent>
	{
		Vec3f position;
		Quatf rotation;
		Vec3f scale;

		TransformComponent();
		TransformComponent(const Vec3f& position, const Quatf& rotation, const Vec3f& scale);

		Vec3f GetForward();
		Vec3f GetBackward();
		Vec3f GetLeft();
		Vec3f GetRight();
		Vec3f GetUp();
		Vec3f GetDown();

		Mat4f GetMatrix();
	};
}