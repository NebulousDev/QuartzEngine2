#include "Physics.h"

#define PHYSICS_SMALLEST_DISTANCE 0.0001f

namespace Quartz
{
	using CollisionFunc = Collision(*)(const Collider& collider0, const Collider& collider1);

	Collision Physics::CollideSphereSphere(const Collider& sphere0, const Collider& sphere1)
	{
		Vec3f diff		= sphere1.transform.position - sphere0.transform.position;
		float scale0	= sphere0.transform.scale.Maximum();
		float scale1	= sphere1.transform.scale.Maximum();
		float radius0	= scale0 * sphere0.sphere.radius;
		float radius1	= scale1 * sphere1.sphere.radius;

		float dist = diff.Magnitude();

		if (dist > PHYSICS_SMALLEST_DISTANCE && dist < (radius0 + radius1))
		{
			Vec3f normDir = diff.Normalized();
			Vec3f extent0 = sphere0.transform.position + normDir * radius0;
			Vec3f extent1 = sphere1.transform.position - normDir * radius1;

			return Collision(extent0, extent1);
		}
			
		return Collision(); // No Collision
	};

	Collision Physics::CollideSpherePlane(const Collider& sphere0, const Collider& plane1)
	{
		float scale0		= sphere0.transform.scale.Maximum();
		float radius0		= scale0 * sphere0.sphere.radius;
		Vec3f rotNormal		= plane1.transform.rotation * plane1.plane.normal;
		Vec3f planePoint	= rotNormal * plane1.plane.length + plane1.transform.position;

		float dist = Dot(sphere0.transform.position - planePoint, rotNormal);

		if (dist > PHYSICS_SMALLEST_DISTANCE && dist < radius0)
		{
			Vec3f extent0 = sphere0.transform.position - rotNormal * radius0;
			Vec3f extent1 = sphere0.transform.position - rotNormal * dist;

			return Collision(extent0, extent1);
		}

		return Collision(); // No Collision
	}

	Collision Physics::CollideSphereRect(const Collider& sphere0, const Collider& rect1)
	{
		Mat4f rectTransform = rect1.transform.GetMatrix();
		Bounds3f rectBounds = rect1.rect.bounds;

		// Transform Rect in to world space
		//rectBounds.start = rectTransform * rectBounds.start;
		//rectBounds.end = rectTransform * rectBounds.end;

		Vec3f points[8]
		{
			rectTransform * rectBounds.BottomRightFront(),
			rectTransform * rectBounds.BottomLeftFront(),
			rectTransform * rectBounds.BottomRightBack(),
			rectTransform * rectBounds.BottomLeftBack(),
			rectTransform * rectBounds.TopRightFront(),
			rectTransform * rectBounds.TopLeftFront(),
			rectTransform * rectBounds.TopRightBack(),
			rectTransform * rectBounds.TopLeftBack()
		};

		Simplex simplex;

		if (GJKRect(sphere0, rect1, points, simplex))
		{
			return EPA(sphere0, rect1, simplex);
		}

		return Collision(); // No Collision
	}

	Collision Physics::CollideSphereCapsule(const Collider& sphere0, const Collider& capsule1)
	{
		return Collision(); // No Collision
	}

	Collision Physics::CollideSphereHull(const Collider& sphere0, const Collider& hull1)
	{
		return Collision(); // No Collision
	}

	Collision Physics::CollideSphereMesh(const Collider& sphere0, const Collider& mesh1)
	{
		return Collision(); // No Collision
	}

	Collision Physics::CollidePlaneSphere(const Collider& plane0, const Collider& sphere1)
	{
		return CollideSpherePlane(sphere1, plane0).Flip();
	};

	Collision Physics::CollidePlanePlane(const Collider& plane0, const Collider& plane1)
	{
		return Collision(); // No Collision
	}

	Collision Physics::CollidePlaneRect(const Collider& plane0, const Collider& rect1)
	{
		Vec3f rotNormal = plane0.transform.rotation * plane0.plane.normal;
		Vec3f planePoint = rotNormal * plane0.plane.length + plane0.transform.position;

		Vec3f points[8]
		{
			rect1.rect.bounds.BottomRightFront(),
			rect1.rect.bounds.BottomLeftFront(),
			rect1.rect.bounds.BottomRightBack(),
			rect1.rect.bounds.BottomLeftBack(),
			rect1.rect.bounds.TopRightFront(),
			rect1.rect.bounds.TopLeftFront(),
			rect1.rect.bounds.TopRightBack(),
			rect1.rect.bounds.TopLeftBack()
		};

		float maxDist = -1000000.0f;
		Vec3f maxPoint;

		for (uSize i = 0; i < 8; i++)
		{
			Vec3f point = rect1.transform.GetMatrix() * points[i];
			float dist = Dot(point, -rotNormal);

			if (dist > maxDist)
			{
				maxDist = dist;
				maxPoint = point;
			}
		}

		Vec3f diff = planePoint - maxPoint;
		float dist = Dot(diff, rotNormal);

		if (dist > PHYSICS_SMALLEST_DISTANCE)
		{
			Vec3f extent0 = maxPoint + rotNormal * dist / 2.0f;
			Vec3f extent1 = maxPoint - rotNormal * dist / 2.0f;

			return Collision(extent0, extent1);
		}

		return Collision(); // No Collision
	}

	Collision Physics::CollidePlaneCapsule(const Collider& plane0, const Collider& capsule1)
	{
		return Collision(); // No Collision
	}

	Collision Physics::CollidePlaneHull(const Collider& plane0, const Collider& hull1)
	{
		return Collision(); // No Collision
	}

	Collision Physics::CollidePlaneMesh(const Collider& plane0, const Collider& mesh1)
	{
		return Collision(); // No Collision
	}

	Collision Physics::CollideRectSphere(const Collider& rect0, const Collider& sphere1)
	{ 
		return CollideSphereRect(sphere1, rect0).Flip();
	};

	Collision Physics::CollideRectPlane(const Collider& rect0, const Collider& plane1)
	{
		return CollidePlaneRect(plane1, rect0).Flip();
	};

	Collision Physics::CollideRectRect(const Collider& rect0, const Collider& rect1)
	{
		return Collision(); // No Collision
	}

	Collision Physics::CollideRectCapsule(const Collider& rect0, const Collider& capsule1)
	{
		return Collision(); // No Collision
	}

	Collision Physics::CollideRectHull(const Collider& rect0, const Collider& hull1)
	{
		return Collision(); // No Collision
	}

	Collision Physics::CollideRectMesh(const Collider& rect0, const Collider& mesh1)
	{
		return Collision(); // No Collision
	}

	Collision Physics::CollideCapsuleSphere(const Collider& capsule0, const Collider& sphere1)
	{
		return CollideSphereCapsule(sphere1, capsule0).Flip();
	};

	Collision Physics::CollideCapsulePlane(const Collider& capsule0, const Collider& plane1)
	{
		return CollidePlaneCapsule(plane1, capsule0).Flip();
	};

	Collision Physics::CollideCapsuleRect(const Collider& capsule0, const Collider& rect1)
	{
		return CollideRectCapsule(rect1, capsule0).Flip();
	};

	Collision Physics::CollideCapsuleCapsule(const Collider& capsule0, const Collider& capsule1)
	{
		return Collision(); // No Collision
	}

	Collision Physics::CollideCapsuleHull(const Collider& capsule0, const Collider& hull1)
	{
		return Collision(); // No Collision
	}

	Collision Physics::CollideCapsuleMesh(const Collider& capsule0, const Collider& mesh1)
	{
		return Collision(); // No Collision
	}

	Collision Physics::CollideHullSphere(const Collider& hull0, const Collider& sphere1)
	{
		return CollideSphereHull(sphere1, hull0).Flip();
	};

	Collision Physics::CollideHullPlane(const Collider& hull0, const Collider& plane1)
	{
		return CollidePlaneHull(plane1, hull0).Flip();
	};

	Collision Physics::CollideHullRect(const Collider& hull0, const Collider& rect1)
	{
		return CollideRectHull(rect1, hull0).Flip();
	};

	Collision Physics::CollideHullCapsule(const Collider& hull0, const Collider& capsule1)
	{
		return CollideCapsuleHull(capsule1, hull0).Flip();
	};

	Collision Physics::CollideHullHull(const Collider& hull0, const Collider& hull1)
	{
		return Collision(); // No Collision
	}

	Collision Physics::CollideHullMesh(const Collider& hull0, const Collider& mesh1)
	{
		return Collision(); // No Collision
	}

	Collision Physics::CollideMeshSphere(const Collider& mesh0, const Collider& sphere1)
	{
		return CollideSphereMesh(sphere1, mesh0).Flip();
	};

	Collision Physics::CollideMeshPlane(const Collider& mesh0, const Collider& plane1)
	{
		return CollidePlaneMesh(plane1, mesh0).Flip();
	};

	Collision Physics::CollideMeshRect(const Collider& mesh0, const Collider& rect1)
	{
		return CollideRectMesh(rect1, mesh0).Flip();
	};

	Collision Physics::CollideMeshCapsule(const Collider& mesh0, const Collider& capsule1)
	{ 
		return CollideCapsuleMesh(capsule1, mesh0).Flip();
	};

	Collision Physics::CollideMeshHull(const Collider& mesh0, const Collider& hull1)
	{
		return CollideHullMesh(hull1, mesh0).Flip();
	}

	Collision Physics::CollideMeshMesh(const Collider& mesh0, const Collider& mesh1)
	{
		return Collision(); // No Collision
	}

	Collision Physics::Collide(const Collider& collider0, const Collider& collider1)
	{
		static CollisionFunc functionTable[36]
		{
			CollideSphereSphere,
			CollideSpherePlane,
			CollideSphereRect,
			CollideSphereCapsule,
			CollideSphereHull,
			CollideSphereMesh,

			CollidePlaneSphere,
			CollidePlanePlane,
			CollidePlaneRect,
			CollidePlaneCapsule,
			CollidePlaneHull,
			CollidePlaneMesh,

			CollideRectSphere,
			CollideRectPlane,
			CollideRectRect,
			CollideRectCapsule,
			CollideRectHull,
			CollideRectMesh,

			CollideCapsuleSphere,
			CollideCapsulePlane,
			CollideCapsuleRect,
			CollideCapsuleCapsule,
			CollideCapsuleHull,
			CollideCapsuleMesh,

			CollideHullSphere,
			CollideHullPlane,
			CollideHullRect,
			CollideHullCapsule,
			CollideHullHull,
			CollideHullMesh,

			CollideMeshSphere,
			CollideMeshPlane,
			CollideMeshRect,
			CollideMeshCapsule,
			CollideMeshHull,
			CollideMeshMesh
		};

		uSize index = (uSize)collider1.shape + ((uSize)collider0.shape * 6);

		return functionTable[index](collider0, collider1);
	}
}