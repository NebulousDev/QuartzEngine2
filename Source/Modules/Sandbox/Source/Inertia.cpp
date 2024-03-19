#include "Physics.h"

namespace Quartz
{
	using InitalInertiaFunc = Vec3f(*)(const RigidBody& rigidBody, const Collider& collider);

	Vec3f Physics::InitalInertiaSphere(const RigidBody& rigidBody, const Collider& sphere)
	{
		if (rigidBody.invMass != 0.0f)
		{
			return Vec3f((5.0f / 3.0f * rigidBody.invMass) * sphere.sphere.radius * sphere.transform.scale * 2);
		}

		return Vec3f(0, 0, 0);
	}

	Vec3f Physics::InitalInertiaPlane(const RigidBody& rigidBody, const Collider& plane)
	{
		if (rigidBody.invMass != 0.0f)
		{
			return Vec3f((1.0f / 12.0f * rigidBody.invMass) *
				(plane.plane.width * plane.plane.width +
					plane.plane.height * plane.plane.height));
		}

		return Vec3f(0, 0, 0);
	}

	Vec3f Physics::InitalInertiaRect(const RigidBody& rigidBody, const Collider& rect)
	{
		if (rigidBody.invMass != 0.0f)
		{
			float width		= rect.rect.bounds.Width() * rect.transform.scale.x;
			float height	= rect.rect.bounds.Height() * rect.transform.scale.y;
			float depth		= rect.rect.bounds.Depth() * rect.transform.scale.z;

			float ix = (1.0f / 12.0f * rigidBody.invMass) * (width * width + depth * depth);
			float iy = (1.0f / 12.0f * rigidBody.invMass) * (depth * depth + height * height);
			float iz = (1.0f / 12.0f * rigidBody.invMass) * (width * width + height * height);

			return Vec3f(ix, iy, iz);
		}
		
		return Vec3f(0, 0, 0);
	}

	Vec3f Physics::InitalInertiaCapsule(const RigidBody& rigidBody, const Collider& capsule)
	{
		return Vec3f(1.0f);
	}

	Vec3f Physics::InitalInertiaHull(const RigidBody& rigidBody, const Collider& hull)
	{
		return Vec3f(1.0f);
	}

	Vec3f Physics::InitalInertiaMesh(const RigidBody& rigidBody, const Collider& mesh)
	{
		return Vec3f(1.0f);
	}

	Vec3f Physics::InitalInertia(const RigidBody& rigidBody, const Collider& collider)
	{
		static InitalInertiaFunc functionTable[6]
		{
			InitalInertiaSphere,
			InitalInertiaPlane,
			InitalInertiaRect,
			InitalInertiaCapsule,
			InitalInertiaHull,
			InitalInertiaMesh
		};

		return functionTable[(uSize)collider.shape](rigidBody, collider);
	}
}