#pragma once

#include "Math/Math.h"
#include "Entity/Component.h"
#include "Colliders.h"

namespace Quartz
{
	struct RigidBodyComponent : public Component<RigidBodyComponent>
	{
		float		friction;
		float		mass;
		Vec3f		gravity;
		Vec3f		velocity;
		Vec3f		force;
		Collider	collider;

		inline RigidBodyComponent()
		{
			friction = 1.0f;
			mass = 1.0f;
			gravity = { 0.0f, -9.81f, 0.0f };
			velocity = { 0.0f, 0.0f, 0.0f };
			force = { 0.0f, 0.0f, 0.0f };
		}

		inline void AddForce(const Vec3f& push)
		{
			force += push;
		}
	};
}