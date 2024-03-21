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
		Vec3f torque;
		Vec3f linearVelocity;
		Vec3f angularVelocity;
		Mat3f invInertiaTensor;
		bool  asleep;

		Vec3f lastAcceleration;

		inline RigidBody() :
			invMass(1.0f),
			restitution(0.5f),
			friction(0.5f),
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

		inline void AddTorque(const Vec3f& torque)
		{
			this->torque += torque;
		}

		inline void AddLinearVelocity(const Vec3f& velocity)
		{
			this->linearVelocity += velocity;
		}

		inline void AddAngularVelocity(const Vec3f& velocity)
		{
			this->angularVelocity += velocity;
		}
	};
}