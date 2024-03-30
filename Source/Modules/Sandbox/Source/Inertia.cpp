#include "Physics.h"

namespace Quartz
{
	Vec3p Physics::InitalInertiaSphere(const RigidBody& rigidBody, const SphereCollider& sphere, const Vec3p& scale)
	{
		if (rigidBody.invMass != 0.0f)
		{
			return Vec3p((2.0f / 5.0f) * (1.0f / rigidBody.invMass) * sphere.GetSphere().radius * scale.Maximum());
		}

		return Vec3p(0, 0, 0);
	}

	Vec3p Physics::InitalInertiaPlane(const RigidBody& rigidBody, const PlaneCollider& plane, const Vec3p& scale)
	{
		if (rigidBody.invMass != 0.0f)
		{
			return Vec3p((1.0f / 12.0f) * (1.0f / rigidBody.invMass) *
				(plane.GetPlane().width * plane.GetPlane().width +
					plane.GetPlane().height * plane.GetPlane().height));
		}

		return Vec3p(0, 0, 0);
	}

	Vec3p Physics::InitalInertiaRect(const RigidBody& rigidBody, const RectCollider& rect, const Vec3p& scale)
	{
		if (rigidBody.invMass != 0.0f)
		{
			floatp width	= rect.GetRect().bounds.Width(); //* scale.x;
			floatp height	= rect.GetRect().bounds.Height(); //* scale.y;
			floatp depth	= rect.GetRect().bounds.Depth(); //* scale.z;

			floatp ix = (1.0f / 12.0f) * (1.0f / rigidBody.invMass) * (depth * depth + height * height);
			floatp iy = (1.0f / 12.0f) * (1.0f / rigidBody.invMass) * (width * width + depth * depth);
			floatp iz = (1.0f / 12.0f) * (1.0f / rigidBody.invMass) * (width * width + height * height);

			return Vec3p(ix, iy, iz);
			//return Vec3p(1, 1, 1);
		}
		
		return Vec3p(0, 0, 0);
	}

	Vec3p Physics::InitalInertiaCapsule(const RigidBody& rigidBody, const CapsuleCollider& capsule, const Vec3p& scale)
	{
		return Vec3p(1.0f);
	}

	Vec3p Physics::InitalInertiaHull(const RigidBody& rigidBody, const HullCollider& hull, const Vec3p& scale)
	{
		return Vec3p(1.0f);
	}

	Vec3p Physics::InitalInertiaMesh(const RigidBody& rigidBody, const MeshCollider& mesh, const Vec3p& scale)
	{
		return Vec3p(1.0f);
	}

	Vec3p Physics::InitalInertia(const RigidBody& rigidBody, const Collider& collider, const Vec3p& scale)
	{
		using InitalInertiaFunc = Vec3p(*)(const RigidBody& rigidBody, const Collider& collider, const Vec3p& scale);

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