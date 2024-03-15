#pragma once

#include "RigidBody.h"
#include "Math/Math.h"
#include "Entity/Component.h"
#include "Colliders.h"

namespace Quartz
{
	struct RigidBodyComponent : public Component<RigidBodyComponent>
	{
		Transform	transform;
		RigidBody	rigidBody;
		Collider	collider;

		inline RigidBodyComponent() {}

		inline RigidBodyComponent(const RigidBody& rigidBody, const Collider& collider) :
			rigidBody(rigidBody), collider(collider) {}

		inline void AddForce(const Vec3f& force)
		{
			rigidBody.AddForce(force);
		}
	};
}