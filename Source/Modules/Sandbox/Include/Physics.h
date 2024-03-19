#pragma once

#include "Engine.h"
#include "Colliders.h"
#include "Entity/World.h"
#include "Component/PhysicsComponent.h"
#include "Component/TransformComponent.h"

#define PHYSICS_GJK_MAX_ITERATIONS	32
#define PHYSICS_EPA_MAX_ITERATIONS	32
#define PHYSICS_EPA_MAX_EDGES		256 //1536
#define PHYSICS_EPA_MAX_TRIS		64 //256
//#define PHYSICS_EPA_MAX_TETRAS	64
#define PHYSICS_STEP_ITERATIONS		16
#define PHYSICS_SMALLEST_DISTANCE	0.0001f

namespace Quartz
{
	class Physics
	{
	public:
		using RigidBodyView = EntityView<RigidBodyComponent, TransformComponent>;

	public:

		struct CollisionData
		{
			RigidBodyComponent* pRigidBody0;
			RigidBodyComponent* pRigidBody1;
			TransformComponent* pTransform0;
			TransformComponent* pTransform1;

			Collision collision;
		};

		struct Line
		{
			Vec3f points[2];

			Line();
			Line(const Vec3f& a, const Vec3f& b);
		};

		struct Triangle
		{
			Vec3f points[3];

			Triangle();
			Triangle(const Vec3f& a, const Vec3f& b, const Vec3f& c);
		};

		struct Tetrahedron
		{
			Vec3f points[4];

			Tetrahedron();
			Tetrahedron(const Vec3f& a, const Vec3f& b, const Vec3f& c, const Vec3f& d);
		};

		class Polytope
		{
		private:
			Triangle	mTris[PHYSICS_EPA_MAX_TRIS];
			uSize		mSize;

		public:
			Polytope();

			inline bool AddSimplex(const Simplex& simplex);
			inline void AddTriangle(const Triangle& tri);
			inline void RemoveTriangle(uSize index);
			inline void ClosestTriangle(const Vec3f& point, Triangle& outTri, 
				uSize& outIndex, float& outDist, Vec3f& outNormal) const;
			inline void Extend(const Vec3f& point);
		};

	private:
		Array<CollisionData> mCollisions;

	private:

		/* Point Detection */

		static Vec3f FurthestPointSphere(const Collider& sphere, const Vec3f& direction);
		static Vec3f FurthestPointPlane(const Collider& plane, const Vec3f& direction);
		static Vec3f FurthestPointRect(const Collider& rect, const Vec3f& direction, Vec3f(&points)[8]);
		static Vec3f FurthestPointRect(const Collider& rect, const Vec3f& direction);
		static Vec3f FurthestPointCapsule(const Collider& capsule, const Vec3f& direction);
		static Vec3f FurthestPointHull(const Collider& hull, const Vec3f& direction);
		static Vec3f FurthestPointMesh(const Collider& mesh, const Vec3f& direction);
		static Vec3f FurthestPoint(const Collider& collider0, const Vec3f& direction);

		static Simplex FurthestSimplexSphere(const Collider& sphere, const Vec3f& direction);
		static Simplex FurthestSimplexPlane(const Collider& plane, const Vec3f& direction);
		static Simplex FurthestSimplexRect(const Collider& rect, const Vec3f& direction, Vec3f(&points)[8]);
		static Simplex FurthestSimplexRect(const Collider& rect, const Vec3f& direction);
		static Simplex FurthestSimplexCapsule(const Collider& capsule, const Vec3f& direction);
		static Simplex FurthestSimplexHull(const Collider& hull, const Vec3f& direction);
		static Simplex FurthestSimplexMesh(const Collider& mesh, const Vec3f& direction);
		static Simplex FurthestSimplex(const Collider& collider0, const Vec3f& direction);

		/* Default Inertia */

		static Vec3f InitalInertiaSphere(const RigidBody& rigidBody, const Collider& sphere);
		static Vec3f InitalInertiaPlane(const RigidBody& rigidBody, const Collider& plane);
		static Vec3f InitalInertiaRect(const RigidBody& rigidBody, const Collider& rect);
		static Vec3f InitalInertiaCapsule(const RigidBody& rigidBody, const Collider& capsule);
		static Vec3f InitalInertiaHull(const RigidBody& rigidBody, const Collider& hull);
		static Vec3f InitalInertiaMesh(const RigidBody& rigidBody, const Collider& mesh);
		static Vec3f InitalInertia(const RigidBody& rigidBody, const Collider& collider);

		/* GJK + EPA */

		static Vec3f MinkowskiFurthestPoint(const Collider& collider0, const Collider& collider1, const Vec3f direction);
		static Vec3f MinkowskiFurthestPointRect(const Collider& collider0, const Collider& rect1, Vec3f(&points)[8], const Vec3f direction);
		static Vec3f MinkowskiFurthestPointRectRect(const Collider& rect0, Vec3f(&points0)[8], const Collider& rect1, Vec3f(&points1)[8], const Vec3f direction);

		static bool GJK(const Collider& collider0, const Collider& collider1, Simplex& outSimplex);
		static bool GJKRect(const Collider& collider0, const Collider& rect1, Vec3f(&points)[8], Simplex& outSimplex);
		static bool GJKRectRect(const Collider& rect0, Vec3f(&points0)[8], const Collider& rect1, Vec3f(&points1)[8], Simplex& outSimplex);

		static Collision EPA(const Collider& collider0, const Collider& collider1, const Simplex& simplex);
		static Collision EPARect(const Collider& collider0, const Collider& rect1, Vec3f(&points)[8], const Simplex& simplex);
		static Collision EPARectRect(const Collider& rect0, Vec3f(&points0)[8], const Collider& rect1, Vec3f(&points1)[8], const Simplex& simplex);

		/* Find Collisions */

		static Collision CollideSphereSphere(const Collider& sphere0, const Collider& sphere1);
		static Collision CollideSpherePlane(const Collider& sphere0, const Collider& plane1);
		static Collision CollideSphereRect(const Collider& sphere0, const Collider& rect1);
		static Collision CollideSphereCapsule(const Collider& sphere0, const Collider& capsule1);
		static Collision CollideSphereHull(const Collider& sphere0, const Collider& hull1);
		static Collision CollideSphereMesh(const Collider& sphere0, const Collider& mesh1);
		static Collision CollidePlaneSphere(const Collider& plane0, const Collider& sphere1);
		static Collision CollidePlanePlane(const Collider& plane0, const Collider& plane1);
		static Collision CollidePlaneRect(const Collider& plane0, const Collider& rect1);
		static Collision CollidePlaneCapsule(const Collider& plane0, const Collider& capsule1);
		static Collision CollidePlaneHull(const Collider& plane0, const Collider& hull1);
		static Collision CollidePlaneMesh(const Collider& plane0, const Collider& mesh1);
		static Collision CollideRectSphere(const Collider& rect0, const Collider& sphere1);
		static Collision CollideRectPlane(const Collider& rect0, const Collider& plane1);
		static Collision CollideRectRect(const Collider& rect0, const Collider& rect1);
		static Collision CollideRectCapsule(const Collider& rect0, const Collider& capsule1);
		static Collision CollideRectHull(const Collider& rect0, const Collider& hull1);
		static Collision CollideRectMesh(const Collider& rect0, const Collider& mesh1);
		static Collision CollideCapsuleSphere(const Collider& capsule0, const Collider& sphere1);
		static Collision CollideCapsulePlane(const Collider& capsule0, const Collider& plane1);
		static Collision CollideCapsuleRect(const Collider& capsule0, const Collider& rect1);
		static Collision CollideCapsuleCapsule(const Collider& capsule0, const Collider& capsule1);
		static Collision CollideCapsuleHull(const Collider& capsule0, const Collider& hull1);
		static Collision CollideCapsuleMesh(const Collider& capsule0, const Collider& mesh1);
		static Collision CollideHullSphere(const Collider& hull0, const Collider& sphere1);
		static Collision CollideHullPlane(const Collider& hull0, const Collider& plane1);
		static Collision CollideHullRect(const Collider& hull0, const Collider& rect1);
		static Collision CollideHullCapsule(const Collider& hull0, const Collider& capsule1);
		static Collision CollideHullHull(const Collider& hull0, const Collider& hull1);
		static Collision CollideHullMesh(const Collider& hull0, const Collider& mesh1);
		static Collision CollideMeshSphere(const Collider& mesh0, const Collider& sphere1);
		static Collision CollideMeshPlane(const Collider& mesh0, const Collider& plane1);
		static Collision CollideMeshRect(const Collider& mesh0, const Collider& rect1);
		static Collision CollideMeshCapsule(const Collider& mesh0, const Collider& capsule1);
		static Collision CollideMeshHull(const Collider& mesh0, const Collider& hull1);
		static Collision CollideMeshMesh(const Collider& mesh0, const Collider& mesh1);

		/* Apply Physics */

		void ApplyForces(EntityWorld& world, RigidBodyView& rigidBodies, double stepTime);
		void FindCollisions(EntityWorld& world, RigidBodyView& rigidBodies, double stepTime);
		void ApplyImpulses(EntityWorld& world, RigidBodyView& rigidBodies, double stepTime);

		/* Triggers */

		void OnRigidBodyAdded(Runtime& runtime, const ComponentAddedEvent<RigidBodyComponent>& event);

	public:
		void Initialize();

		static Collision Collide(const Collider& collider0, const Collider& collider1);

		void Step(EntityWorld& world, double deltaTime);
	};
}