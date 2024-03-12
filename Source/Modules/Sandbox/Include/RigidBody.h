#pragma once

#include "Math/Math.h"

namespace Quartz
{
	class RigidBody
	{
		Transform	transform;
		float		invMass;
		Vec3f		linearVelocity;
		Vec3f		angularVelocity;
		float		restitution;			// bouncy-ness
		Mat3f		inverseInertiaTensor;
	};
}