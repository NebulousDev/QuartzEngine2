#pragma once

#include "GfxAPI.h"
#include "Entity/Component.h"
#include "Math/Math.h"

namespace Quartz
{
	struct QUARTZ_GRAPHICS_API TransformComponent : public Component<TransformComponent>
	{
		Vec3f position;
		Quatf rotation;
		Vec3f scale;

		TransformComponent();
		TransformComponent(const Vec3f& position, const Quatf& rotation, const Vec3f& scale);

		Vec3f GetForward() const;
		Vec3f GetBackward() const;
		Vec3f GetLeft() const;
		Vec3f GetRight() const;
		Vec3f GetUp() const;
		Vec3f GetDown() const;

		Mat4f GetMatrix() const;
		Mat4f GetViewMatrix() const;
	};
}