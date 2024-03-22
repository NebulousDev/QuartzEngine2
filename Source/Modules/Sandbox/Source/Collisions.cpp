#include "CollisionDetection.h"
#include "GJK.h"
#include <float.h>

namespace Quartz
{
	bool CollisionDetection::CollideSphereSphere(const SphereCollider& sphere0, const Transform& transform0, 
		const SphereCollider& sphere1, const Transform& transform1, Collision& outCollision)
	{
		const Vec3f& position0	= transform0.position;
		const Vec3f& position1	= transform1.position;
		const float& scale0		= transform0.scale.Maximum();
		const float& scale1		= transform1.scale.Maximum();
		const float radius0		= sphere0.GetSphere().radius * scale0;
		const float radius1		= sphere1.GetSphere().radius * scale1;
		const float totalRadius = radius0 + radius1;

		Vec3f normal = position0 - position1; // Sphere1 to Sphere0
		float dist = normal.Magnitude();

		if (dist > 0.0f && dist < totalRadius) // PHYSICS_SMALLEST_DISTANCE?
		{
			Vec3f contactPoint = position0 + normal * 0.5;
			float depth = totalRadius - dist;

			normal = normal * (1.0f / dist); // Quick normalize

			Collision collision(normal, depth);
			collision.AddContact(contactPoint);

			outCollision = collision;

			return true;
		}
			
		return false; // No Collision
	};

	bool CollisionDetection::CollideSpherePlane(const SphereCollider& sphere0, const Transform& transform0,
		const PlaneCollider& plane1, const Transform& transform1, Collision& outCollision)
	{
		const Vec3f& position0	= transform0.position;
		const Vec3f& position1	= transform1.position;
		const Quatf& rotation1	= transform1.rotation;
		const float& scale0		= transform0.scale.Maximum();
		const float radius0		= sphere0.GetSphere().radius * scale0;
		
		Vec3f normal = rotation1 * plane1.GetPlane().normal; // Should always be normal

		float offset = Dot(normal, position1);
		float dist = Dot(normal, position0) - offset;

		if (dist * dist <= radius0 * radius0) // PHYSICS_SMALLEST_DISTANCE?
		{
			float depth = -dist;

			if (dist < 0)
			{
				normal = -normal;
				depth = -depth;
			}

			depth += radius0;

			Vec3f contactPoint = position0 - normal * dist;

			Collision collision(normal, depth);
			collision.AddContact(contactPoint);

			outCollision = collision;

			return true;
		}

		return false; // No Collision
	}

	bool CollisionDetection::CollideSphereRect(const SphereCollider& sphere0, const Transform& transform0, 
		const RectCollider& rect1, const Transform& transform1, Collision& outCollision)
	{
		const Mat4f& transform00 = transform0.GetMatrix();
		const Mat4f& transform11 = transform1.GetMatrix();
		const Bounds3f& bounds = rect1.GetRect().bounds;

		Vec3f points[8]
		{
			transform11 * bounds.BottomRightFront(),
			transform11 * bounds.BottomLeftFront(),
			transform11 * bounds.BottomRightBack(),
			transform11 * bounds.BottomLeftBack(),
			transform11 * bounds.TopRightFront(),
			transform11 * bounds.TopLeftFront(),
			transform11 * bounds.TopRightBack(),
			transform11 * bounds.TopLeftBack()
		};

		Simplex simplex;

		if (GJK::GJKRect(sphere0.GetSphere(), transform00, rect1.GetRect(), points, simplex))
		{
			//return EPARect(sphere0, rect1, points, simplex);
		}

		return false; // No Collision
	}

	bool CollisionDetection::CollideSphereCapsule(const SphereCollider& sphere0, const Transform& transform0, 
		const CapsuleCollider& capsule1, const Transform& transform1, Collision& outCollision)
	{
		return false; // No Collision
	}

	bool CollisionDetection::CollideSphereHull(const SphereCollider& sphere0, const Transform& transform0, 
		const HullCollider& hull1, const Transform& transform1, Collision& outCollision)
	{
		Simplex simplex;

		//if (GJK(sphere0, hull1, simplex))
		//{
		//	return EPA(sphere0, hull1, simplex);
		//}

		return false; // No Collision
	}

	bool CollisionDetection::CollideSphereMesh(const SphereCollider& sphere0, const Transform& transform0, 
		const MeshCollider& mesh1, const Transform& transform1, Collision& outCollision)
	{
		return false; // No Collision
	}

	bool CollisionDetection::CollidePlaneSphere(const PlaneCollider& plane0, const Transform& transform0, 
		const SphereCollider& sphere1, const Transform& transform1, Collision& outCollision)
	{
		bool result = CollideSpherePlane(sphere1, transform1, plane0, transform0, outCollision);
		outCollision.Flip();
		return result;
	};

	bool CollisionDetection::CollidePlanePlane(const PlaneCollider& plane0, const Transform& transform0, 
		const PlaneCollider& plane1, const Transform& transform1, Collision& outCollision)
	{
		return false; // No Collision
	}

	bool CollisionDetection::CollidePlaneRect(const PlaneCollider& plane0, const Transform& transform0, 
		const RectCollider& rect1, const Transform& transform1, Collision& outCollision)
	{
		const Vec3f& position0	= transform0.position;
		const Vec3f& position1	= transform1.position;
		const Vec3f rotNormal	= transform0.rotation * -plane0.GetPlane().normal;
		const Vec3f planePoint	= rotNormal * plane0.GetPlane().length + position0;

		const Mat4f& transform = transform1.GetMatrix();
		const Bounds3f& bounds = rect1.GetRect().bounds;

		Vec3f points[8]
		{
			transform * bounds.BottomRightFront(),
			transform * bounds.BottomLeftFront(),
			transform * bounds.BottomRightBack(),
			transform * bounds.BottomLeftBack(),
			transform * bounds.TopRightFront(),
			transform * bounds.TopLeftFront(),
			transform * bounds.TopRightBack(),
			transform * bounds.TopLeftBack()
		};

		float minDist = -FLT_MAX;
		Vec3f minPoint;

		float offset = Dot(rotNormal, position0);
		
		for (uSize i = 0; i < 8; i++)
		{
			float dist = Dot(rotNormal, points[i]) - offset;

			if (dist > minDist)
			{
				minDist = dist;
				minPoint = points[i];
			}
		}

		if (minDist > 0.0f) //PHYSICS_SMALLEST_DISTANCE?
		{
			Collision collision(-rotNormal, minDist);
			collision.AddContact(minPoint);

			outCollision = collision;
			return true;
		}

		return false; // No Collision
	}

	bool CollisionDetection::CollidePlaneCapsule(const PlaneCollider& plane0, const Transform& transform0, 
		const CapsuleCollider& capsule1, const Transform& transform1, Collision& outCollision)
	{
		return false; // No Collision
	}

	bool CollisionDetection::CollidePlaneHull(const PlaneCollider& plane0, const Transform& transform0, 
		const HullCollider& hull1, const Transform& transform1, Collision& outCollision)
	{
		Simplex simplex;

		//if (GJK(plane0, hull1, simplex))
		//{
		//	return EPA(plane0, hull1, simplex, outCollision);
		//}

		return false; // No Collision
	}

	bool CollisionDetection::CollidePlaneMesh(const PlaneCollider& plane0, const Transform& transform0, const MeshCollider& mesh1, 
		const Transform& transform1, Collision& outCollision)
	{
		return false; // No Collision
	}

	bool CollisionDetection::CollideRectSphere(const RectCollider& rect0, const Transform& transform0, 
		const SphereCollider& sphere1, const Transform& transform1, Collision& outCollision)
	{ 
		bool result = CollideSphereRect(sphere1, transform1, rect0, transform0, outCollision);
		outCollision.Flip();
		return result;
	};

	bool CollisionDetection::CollideRectPlane(const RectCollider& rect0, const Transform& transform0, 
		const PlaneCollider& plane1, const Transform& transform1, Collision& outCollision)
	{
		bool result = CollidePlaneRect(plane1, transform1, rect0, transform0, outCollision);
		outCollision.Flip();
		return result;
	};

	bool CollisionDetection::CollideRectRect(const RectCollider& rect0, const Transform& transform0,
		const RectCollider& rect1, const Transform& transform1, Collision& outCollision)
	{
		const Mat4f& transform00 = transform0.GetMatrix();
		const Mat4f& transform11 = transform1.GetMatrix();
		const Bounds3f& bounds0 = rect1.GetRect().bounds;
		const Bounds3f& bounds1 = rect1.GetRect().bounds;

		Vec3f points0[8]
		{
			transform00 * bounds0.BottomRightFront(),
			transform00 * bounds0.BottomLeftFront(),
			transform00 * bounds0.BottomRightBack(),
			transform00 * bounds0.BottomLeftBack(),
			transform00 * bounds0.TopRightFront(),
			transform00 * bounds0.TopLeftFront(),
			transform00 * bounds0.TopRightBack(),
			transform00 * bounds0.TopLeftBack()
		};

		Vec3f points1[8]
		{
			transform11 * bounds1.BottomRightFront(),
			transform11 * bounds1.BottomLeftFront(),
			transform11 * bounds1.BottomRightBack(),
			transform11 * bounds1.BottomLeftBack(),
			transform11 * bounds1.TopRightFront(),
			transform11 * bounds1.TopLeftFront(),
			transform11 * bounds1.TopRightBack(),
			transform11 * bounds1.TopLeftBack()
		};

		Simplex simplex;

		if (GJK::GJKRectRect(rect0.GetRect(), points0, rect1.GetRect(), points1, simplex))
		{
			return GJK::EPARectRect(rect0.GetRect(), points0, rect1.GetRect(), points1, simplex, outCollision);
		}

		return false; // No Collision
	}

	bool CollisionDetection::CollideRectCapsule(const RectCollider& rect0, const Transform& transform0, 
		const CapsuleCollider& capsule1, const Transform& transform1, Collision& outCollision)
	{
		return false; // No Collision
	}

	bool CollisionDetection::CollideRectHull(const RectCollider& rect0, const Transform& transform0, 
		const HullCollider& hull1, const Transform& transform1, Collision& outCollision)
	{
		const Mat4f& transform00 = transform0.GetMatrix();
		const Mat4f& transform11 = transform1.GetMatrix();
		const Bounds3f& bounds = rect0.GetRect().bounds;

		Vec3f points[8]
		{
			transform00 * bounds.BottomRightFront(),
			transform00 * bounds.BottomLeftFront(),
			transform00 * bounds.BottomRightBack(),
			transform00 * bounds.BottomLeftBack(),
			transform00 * bounds.TopRightFront(),
			transform00 * bounds.TopLeftFront(),
			transform00 * bounds.TopRightBack(),
			transform00 * bounds.TopLeftBack()
		};

		Simplex simplex;

		if (GJK::GJKRect(hull1.GetHull(), transform11, rect0.GetRect(), points, simplex))
		{
			bool result = GJK::EPARect(hull1.GetHull(), transform11, rect0.GetRect(), points, simplex, outCollision);
			outCollision.Flip();
			return result;
		}

		return false; // No Collision
	}

	bool CollisionDetection::CollideRectMesh(const RectCollider& rect0, const Transform& transform0,
		const MeshCollider& mesh1, const Transform& transform1, Collision& outCollision)
	{
		return false; // No Collision
	}

	bool CollisionDetection::CollideCapsuleSphere(const CapsuleCollider& capsule0, const Transform& transform0,
		const SphereCollider& sphere1, const Transform& transform1, Collision& outCollision)
	{
		bool result = CollideSphereCapsule(sphere1, transform1, capsule0, transform0, outCollision);
		outCollision.Flip();
		return result;
	};

	bool CollisionDetection::CollideCapsulePlane(const CapsuleCollider& capsule0, const Transform& transform0,
		const PlaneCollider& plane1, const Transform& transform1, Collision& outCollision)
	{
		bool result = CollidePlaneCapsule(plane1, transform1, capsule0, transform0, outCollision);
		outCollision.Flip();
		return result;
	};

	bool CollisionDetection::CollideCapsuleRect(const CapsuleCollider& capsule0, const Transform& transform0,
		const RectCollider& rect1, const Transform& transform1, Collision& outCollision)
	{
		bool result = CollideRectCapsule(rect1, transform1, capsule0, transform0, outCollision);
		outCollision.Flip();
		return result;
	};

	bool CollisionDetection::CollideCapsuleCapsule(const CapsuleCollider& capsule0, const Transform& transform0,
		const CapsuleCollider& capsule1, const Transform& transform1, Collision& outCollision)
	{
		return false; // No Collision
	}

	bool CollisionDetection::CollideCapsuleHull(const CapsuleCollider& capsule0, const Transform& transform0,
		const HullCollider& hull1, const Transform& transform1, Collision& outCollision)
	{
		return false; // No Collision
	}

	bool CollisionDetection::CollideCapsuleMesh(const CapsuleCollider& capsule0, const Transform& transform0,
		const MeshCollider& mesh1, const Transform& transform1, Collision& outCollision)
	{
		return false; // No Collision
	}

	bool CollisionDetection::CollideHullSphere(const HullCollider& hull0, const Transform& transform0,
		const SphereCollider& sphere1, const Transform& transform1, Collision& outCollision)
	{
		bool result = CollideSphereHull(sphere1, transform1, hull0, transform0, outCollision);
		outCollision.Flip();
		return result;
	};

	bool CollisionDetection::CollideHullPlane(const HullCollider& hull0, const Transform& transform0,
		const PlaneCollider& plane1, const Transform& transform1, Collision& outCollision)
	{
		bool result = CollidePlaneHull(plane1, transform1, hull0, transform0, outCollision);
		outCollision.Flip();
		return result;
	};

	bool CollisionDetection::CollideHullRect(const HullCollider& hull0, const Transform& transform0,
		const RectCollider& rect1, const Transform& transform1, Collision& outCollision)
	{
		bool result = CollideRectHull(rect1, transform1, hull0, transform0, outCollision);
		outCollision.Flip();
		return result;
	};

	bool CollisionDetection::CollideHullCapsule(const HullCollider& hull0, const Transform& transform0,
		const CapsuleCollider& capsule1, const Transform& transform1, Collision& outCollision)
	{
		bool result = CollideCapsuleHull(capsule1, transform1, hull0, transform0, outCollision);
		outCollision.Flip();
		return result;
	};

	bool CollisionDetection::CollideHullHull(const HullCollider& hull0, const Transform& transform0,
		const HullCollider& hull1, const Transform& transform1, Collision& outCollision)
	{
		Simplex simplex;

		const Mat4f& transform00 = transform0.GetMatrix();
		const Mat4f& transform11 = transform1.GetMatrix();

		if (GJK::GJK(hull0.GetHull(), transform00, hull1.GetHull(), transform11, simplex))
		{
			return GJK::EPA(hull0.GetHull(), transform00, hull1.GetHull(), transform11, simplex, outCollision);
		}

		return false; // No Collision
	}

	bool CollisionDetection::CollideHullMesh(const HullCollider& hull0, const Transform& transform0,
		const MeshCollider& mesh1, const Transform& transform1, Collision& outCollision)
	{
		return false; // No Collision
	}

	bool CollisionDetection::CollideMeshSphere(const MeshCollider& mesh0, const Transform& transform0,
		const SphereCollider& sphere1, const Transform& transform1, Collision& outCollision)
	{
		bool result = CollideSphereMesh(sphere1, transform1, mesh0, transform0, outCollision);
		outCollision.Flip();
		return result;
	};

	bool CollisionDetection::CollideMeshPlane(const MeshCollider& mesh0, const Transform& transform0,
		const PlaneCollider& plane1, const Transform& transform1, Collision& outCollision)
	{
		bool result = CollidePlaneMesh(plane1, transform1, mesh0, transform0, outCollision);
		outCollision.Flip();
		return result;
	};

	bool CollisionDetection::CollideMeshRect(const MeshCollider& mesh0, const Transform& transform0,
		const RectCollider& rect1, const Transform& transform1, Collision& outCollision)
	{
		bool result = CollideRectMesh(rect1, transform1, mesh0, transform0, outCollision);
		outCollision.Flip();
		return result;
	};

	bool CollisionDetection::CollideMeshCapsule(const MeshCollider& mesh0, const Transform& transform0,
		const CapsuleCollider& capsule1, const Transform& transform1, Collision& outCollision)
	{ 
		bool result = CollideCapsuleMesh(capsule1, transform1, mesh0, transform0, outCollision);
		outCollision.Flip();
		return result;
	};

	bool CollisionDetection::CollideMeshHull(const MeshCollider& mesh0, const Transform& transform0,
		const HullCollider& hull1, const Transform& transform1, Collision& outCollision)
	{
		bool result = CollideHullMesh(hull1, transform1, mesh0, transform0, outCollision);
		outCollision.Flip();
		return result;
	}

	bool CollisionDetection::CollideMeshMesh(const MeshCollider& mesh0, const Transform& transform0,
		const MeshCollider& mesh1, const Transform& transform1, Collision& outCollision)
	{
		return false; // No Collision
	}

	bool CollisionDetection::Collide(const Collider& collider0, const Transform& transform0, 
		const Collider& collider1, const Transform& transform1, Collision& outCollision)
	{
		using CollisionFunc = bool(*)(const Collider& collider0, const Transform& transform0, 
			const Collider& collider1, const Transform& transform1, Collision& outCollision);

		static CollisionFunc functionTable[36]
		{
			(CollisionFunc) CollideSphereSphere,
			(CollisionFunc) CollideSpherePlane,
			(CollisionFunc) CollideSphereRect,
			(CollisionFunc) CollideSphereCapsule,
			(CollisionFunc) CollideSphereHull,
			(CollisionFunc) CollideSphereMesh,

			(CollisionFunc) CollidePlaneSphere,
			(CollisionFunc) CollidePlanePlane,
			(CollisionFunc) CollidePlaneRect,
			(CollisionFunc) CollidePlaneCapsule,
			(CollisionFunc) CollidePlaneHull,
			(CollisionFunc) CollidePlaneMesh,

			(CollisionFunc) CollideRectSphere,
			(CollisionFunc) CollideRectPlane,
			(CollisionFunc) CollideRectRect,
			(CollisionFunc) CollideRectCapsule,
			(CollisionFunc) CollideRectHull,
			(CollisionFunc) CollideRectMesh,

			(CollisionFunc) CollideCapsuleSphere,
			(CollisionFunc) CollideCapsulePlane,
			(CollisionFunc) CollideCapsuleRect,
			(CollisionFunc) CollideCapsuleCapsule,
			(CollisionFunc) CollideCapsuleHull,
			(CollisionFunc) CollideCapsuleMesh,

			(CollisionFunc) CollideHullSphere,
			(CollisionFunc) CollideHullPlane,
			(CollisionFunc) CollideHullRect,
			(CollisionFunc) CollideHullCapsule,
			(CollisionFunc) CollideHullHull,
			(CollisionFunc) CollideHullMesh,

			(CollisionFunc) CollideMeshSphere,
			(CollisionFunc) CollideMeshPlane,
			(CollisionFunc) CollideMeshRect,
			(CollisionFunc) CollideMeshCapsule,
			(CollisionFunc) CollideMeshHull,
			(CollisionFunc) CollideMeshMesh
		};

		const ShapeType& type0 = collider0.GetShapeType();
		const ShapeType& type1 = collider1.GetShapeType();

		// @TODO make debug
		// Improperly formed colliders
		if (type0 == SHAPE_NONE || type1 == SHAPE_NONE)
		{
			return false; // No Collision
		}

		uSize index = (uSize)type1 + ((uSize)type0 * 6);

		return functionTable[index](collider0, transform0, collider1, transform1, outCollision);
	}
}