#pragma once

#include "PhysicsTypes.h"
#include "Collision.h"
#include "Colliders.h"

namespace Quartz
{
	class CollisionDetection
	{
	private:
		static bool CollideSphereSphere(	const SphereCollider& sphere0,		const Transform& transform0, const SphereCollider& sphere1,		const Transform& transform1, Collision& outCollision);
		static bool CollideSpherePlane(		const SphereCollider& sphere0,		const Transform& transform0, const PlaneCollider& plane1,		const Transform& transform1, Collision& outCollision);
		static bool CollideSphereRect(		const SphereCollider& sphere0,		const Transform& transform0, const RectCollider& rect1,			const Transform& transform1, Collision& outCollision);
		static bool CollideSphereCapsule(	const SphereCollider& sphere0,		const Transform& transform0, const CapsuleCollider& capsule1,	const Transform& transform1, Collision& outCollision);
		static bool CollideSphereHull(		const SphereCollider& sphere0,		const Transform& transform0, const HullCollider& hull1,			const Transform& transform1, Collision& outCollision);
		static bool CollideSphereMesh(		const SphereCollider& sphere0,		const Transform& transform0, const MeshCollider& mesh1,			const Transform& transform1, Collision& outCollision);
		static bool CollidePlaneSphere(		const PlaneCollider& plane0,		const Transform& transform0, const SphereCollider& sphere1,		const Transform& transform1, Collision& outCollision);
		static bool CollidePlanePlane(		const PlaneCollider& plane0,		const Transform& transform0, const PlaneCollider& plane1,		const Transform& transform1, Collision& outCollision);
		static bool CollidePlaneRect(		const PlaneCollider& plane0,		const Transform& transform0, const RectCollider& rect1,			const Transform& transform1, Collision& outCollision);
		static bool CollidePlaneCapsule(	const PlaneCollider& plane0,		const Transform& transform0, const CapsuleCollider& capsule1,	const Transform& transform1, Collision& outCollision);
		static bool CollidePlaneHull(		const PlaneCollider& plane0,		const Transform& transform0, const HullCollider& hull1,			const Transform& transform1, Collision& outCollision);
		static bool CollidePlaneMesh(		const PlaneCollider& plane0,		const Transform& transform0, const MeshCollider& mesh1,			const Transform& transform1, Collision& outCollision);
		static bool CollideRectSphere(		const RectCollider& rect0,			const Transform& transform0, const SphereCollider& sphere1,		const Transform& transform1, Collision& outCollision);
		static bool CollideRectPlane(		const RectCollider& rect0,			const Transform& transform0, const PlaneCollider& plane1,		const Transform& transform1, Collision& outCollision);
		static bool CollideRectRect(		const RectCollider& rect0,			const Transform& transform0, const RectCollider& rect1,			const Transform& transform1, Collision& outCollision);
		static bool CollideRectCapsule(		const RectCollider& rect0,			const Transform& transform0, const CapsuleCollider& capsule1,	const Transform& transform1, Collision& outCollision);
		static bool CollideRectHull(		const RectCollider& rect0,			const Transform& transform0, const HullCollider& hull1,			const Transform& transform1, Collision& outCollision);
		static bool CollideRectMesh(		const RectCollider& rect0,			const Transform& transform0, const MeshCollider& mesh1,			const Transform& transform1, Collision& outCollision);
		static bool CollideCapsuleSphere(	const CapsuleCollider& capsule0,	const Transform& transform0, const SphereCollider& sphere1,		const Transform& transform1, Collision& outCollision);
		static bool CollideCapsulePlane(	const CapsuleCollider& capsule0,	const Transform& transform0, const PlaneCollider& plane1,		const Transform& transform1, Collision& outCollision);
		static bool CollideCapsuleRect(		const CapsuleCollider& capsule0,	const Transform& transform0, const RectCollider& rect1,			const Transform& transform1, Collision& outCollision);
		static bool CollideCapsuleCapsule(	const CapsuleCollider& capsule0,	const Transform& transform0, const CapsuleCollider& capsule1,	const Transform& transform1, Collision& outCollision);
		static bool CollideCapsuleHull(		const CapsuleCollider& capsule0,	const Transform& transform0, const HullCollider& hull1,			const Transform& transform1, Collision& outCollision);
		static bool CollideCapsuleMesh(		const CapsuleCollider& capsule0,	const Transform& transform0, const MeshCollider& mesh1,			const Transform& transform1, Collision& outCollision);
		static bool CollideHullSphere(		const HullCollider& hull0,			const Transform& transform0, const SphereCollider& sphere1,		const Transform& transform1, Collision& outCollision);
		static bool CollideHullPlane(		const HullCollider& hull0,			const Transform& transform0, const PlaneCollider& plane1,		const Transform& transform1, Collision& outCollision);
		static bool CollideHullRect(		const HullCollider& hull0,			const Transform& transform0, const RectCollider& rect1,			const Transform& transform1, Collision& outCollision);
		static bool CollideHullCapsule(		const HullCollider& hull0,			const Transform& transform0, const CapsuleCollider& capsule1,	const Transform& transform1, Collision& outCollision);
		static bool CollideHullHull(		const HullCollider& hull0,			const Transform& transform0, const HullCollider& hull1,			const Transform& transform1, Collision& outCollision);
		static bool CollideHullMesh(		const HullCollider& hull0,			const Transform& transform0, const MeshCollider& mesh1,			const Transform& transform1, Collision& outCollision);
		static bool CollideMeshSphere(		const MeshCollider& mesh0,			const Transform& transform0, const SphereCollider& sphere1,		const Transform& transform1, Collision& outCollision);
		static bool CollideMeshPlane(		const MeshCollider& mesh0,			const Transform& transform0, const PlaneCollider& plane1,		const Transform& transform1, Collision& outCollision);
		static bool CollideMeshRect(		const MeshCollider& mesh0,			const Transform& transform0, const RectCollider& rect1,			const Transform& transform1, Collision& outCollision);
		static bool CollideMeshCapsule(		const MeshCollider& mesh0,			const Transform& transform0, const CapsuleCollider& capsule1,	const Transform& transform1, Collision& outCollision);
		static bool CollideMeshHull(		const MeshCollider& mesh0,			const Transform& transform0, const HullCollider& hull1,			const Transform& transform1, Collision& outCollision);
		static bool CollideMeshMesh(		const MeshCollider& mesh0,			const Transform& transform0, const MeshCollider& mesh1,			const Transform& transform1, Collision& outCollision);

	public:
		static bool Collide(const Collider& collider0, const Transform& transform0, 
			const Collider& collider1, const Transform& transform1, Collision& outCollision);
		//static void GenerateContacts(const Collider& collider0, const Collider& collider1, const Collision& collision);
	};
}