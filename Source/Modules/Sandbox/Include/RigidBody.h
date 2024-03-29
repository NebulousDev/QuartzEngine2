#pragma once

#include "PhysicsTypes.h"
#include "Math/Math.h"

namespace Quartz
{
	struct RigidBody
	{
		floatp invMass;
		floatp restitution;	 // bouncy-ness
		floatp friction;
		Vec3p gravity;

		Vec3p force;
		Vec3p torque;
		Vec3p linearVelocity;
		Vec3p angularVelocity;
		Vec3p inertiaVector;
		Mat3p invInertiaTensor;
		bool  asleep;

		Vec3p lastAcceleration;

		inline RigidBody() :
			invMass(1.0f),
			restitution(0.5f),
			friction(0.5f),
			gravity(0.0f, -9.81f, 0.0f) {}

		inline RigidBody(floatp invMass, floatp restitution, floatp friction,
			const Vec3p& gravity = { 0.0f, -9.81f, 0.0f }) :
			invMass(invMass),
			restitution(restitution),
			friction(friction),
			gravity(gravity) {}

		inline void AddForce(const Vec3p& force)
		{
			this->force += force;
		}

		inline void AddTorque(const Vec3p& torque)
		{
			this->torque += torque;
		}

		inline void AddLinearVelocity(const Vec3p& velocity)
		{
			this->linearVelocity += velocity;
		}

		inline void AddAngularVelocity(const Vec3p& velocity)
		{
			this->angularVelocity += velocity;
		}

		inline void UpdateInertia(const Transform& transform)
		{
			if (!inertiaVector.IsZero())
			{
				invInertiaTensor = (Mat3p().SetRotation(transform.rotation) * Mat3p().SetIdentity(inertiaVector));
			}
		}
	};
}