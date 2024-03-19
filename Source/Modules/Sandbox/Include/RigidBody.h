#pragma once

#include "Math/Math.h"

namespace Quartz
{
	struct RigidBody
	{
		float invMass;
		float restitution;	 // bouncy-ness
		float friction;
		Vec3f gravity;

		Vec3f force;
		Vec3f linearVelocity;
		Vec3f angularVelocity;
		Mat3f invInertiaTensor;
		bool  asleep;

		inline RigidBody() :
			invMass(1.0f),
			restitution(1.0f),
			friction(1.0f),
			gravity(0.0f, -9.81f, 0.0f) {}

		inline RigidBody(float invMass, float restitution, float friction,
			const Vec3f& gravity = { 0.0f, -9.81f, 0.0f }) :
			invMass(invMass),
			restitution(restitution),
			friction(friction),
			gravity(gravity) {}

		inline void AddForce(const Vec3f& force)
		{
			this->force += force;
		}
	};
}