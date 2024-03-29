#include "Physics.h"

namespace Quartz
{
	Vec3f Physics::InitalInertiaSphere(const RigidBody& rigidBody, const SphereCollider& sphere, const Vec3f& scale)
	{
		if (rigidBody.invMass != 0.0f)
		{
			return Vec3f((2.0f / 5.0f) * (1.0f / rigidBody.invMass) * sphere.GetSphere().radius * scale.Maximum());
		}

		return Vec3f(0, 0, 0);
	}

	Vec3f Physics::InitalInertiaPlane(const RigidBody& rigidBody, const PlaneCollider& plane, const Vec3f& scale)
	{
		if (rigidBody.invMass != 0.0f)
		{
			return Vec3f((1.0f / 12.0f) * (1.0f / rigidBody.invMass) *
				(plane.GetPlane().width * plane.GetPlane().width +
					plane.GetPlane().height * plane.GetPlane().height));
		}

		return Vec3f(0, 0, 0);
	}

	Vec3f Physics::InitalInertiaRect(const RigidBody& rigidBody, const RectCollider& rect, const Vec3f& scale)
	{
		if (rigidBody.invMass != 0.0f)
		{
			float width		= rect.GetRect().bounds.Width() * scale.x;
			float height	= rect.GetRect().bounds.Height() * scale.y;
			float depth		= rect.GetRect().bounds.Depth() * scale.z;

			float ix = (1.0f / 12.0f) * (1.0f / rigidBody.invMass) * (width * width + depth * depth);
			float iy = (1.0f / 12.0f) * (1.0f / rigidBody.invMass) * (depth * depth + height * height);
			float iz = (1.0f / 12.0f) * (1.0f / rigidBody.invMass) * (width * width + height * height);

			//return Vec3f(ix, iy, iz);
			return Vec3f(iy, iz, ix);
			//return Vec3f(1.0f, 1.0f, 1.0f);
		}
		
		return Vec3f(0, 0, 0);
	}

	Vec3f Physics::InitalInertiaCapsule(const RigidBody& rigidBody, const CapsuleCollider& capsule, const Vec3f& scale)
	{
		return Vec3f(1.0f);
	}

	Vec3f Physics::InitalInertiaHull(const RigidBody& rigidBody, const HullCollider& hull, const Vec3f& scale)
	{
		return Vec3f(1.0f);
	}

	Vec3f Physics::InitalInertiaMesh(const RigidBody& rigidBody, const MeshCollider& mesh, const Vec3f& scale)
	{
		return Vec3f(1.0f);
	}

	Vec3f Physics::InitalInertia(const RigidBody& rigidBody, const Collider& collider, const Vec3f& scale)
	{
		using InitalInertiaFunc = Vec3f(*)(const RigidBody& rigidBody, const Collider& collider, const Vec3f& scale);

		static InitalInertiaFunc functionTable[6]
		{
			(InitalInertiaFunc) InitalInertiaSphere,
			(InitalInertiaFunc) InitalInertiaPlane,
			(InitalInertiaFunc) InitalInertiaRect,
			(InitalInertiaFunc) InitalInertiaCapsule,
			(InitalInertiaFunc) InitalInertiaHull,
			(InitalInertiaFunc) InitalInertiaMesh
		};

		return functionTable[(uSize)collider.GetShapeType()](rigidBody, collider, scale);
	}
}