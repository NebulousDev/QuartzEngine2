#pragma once

#include "GfxAPI.h"
#include "Entity/Component.h"
#include "Math/Math.h"

namespace Quartz
{
	struct QUARTZ_GRAPHICS_API TransformComponent : 
		public Component<TransformComponent>, 
		public Transform
	{
		TransformComponent();
		TransformComponent(const Vec3f& position, const Quatf& rotation, const Vec3f& scale);
	};
}