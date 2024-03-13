#pragma once

#include "Engine.h"
#include "Colliders.h"
#include "Component/PhysicsComponent.h"
#include "Component/TransformComponent.h"

#define PHYSICS_GJK_MAX_ITERATIONS	32
#define PHYSICS_EPA_MAX_ITERATIONS	32
#define PHYSICS_EPA_MAX_TRIANGLES	64

namespace Quartz
{
	class Physics
	{
	public:
		class Simplex
		{
		private:
			Vec3f mPoints[4];
			uSize mSize;

		private:
			inline bool Line(Vec3f& inOutDir);
			inline bool Triangle(Vec3f& inOutDir);
			inline bool Tetrahedron(Vec3f& inOutDir);

		public:
			Simplex();
			
			inline void Push(const Vec3f& point);
			inline uSize Size() const;

			inline bool Next(Vec3f& inOutDir);

			inline const Vec3f& operator[](uSize index) const;
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

		class Polytope
		{
		private:
			Triangle	mTris[PHYSICS_EPA_MAX_TRIANGLES];
			uSize		mSize;

		public:
			Polytope();

			inline bool AddSimplex(const Simplex& simplex);
			inline void AddTriangle(const Triangle& tri);
			inline void RemoveTriangle(uSize index);
			inline void ClosestTriangle(const Vec3f& point, Triangle& outTri, 
				uSize& outIndex, float& outDist, Vec3f& outNormal);
			inline void Extend(const Vec3f& point);
		};

	private:
		static Vec3f FurthestPointSphere(const Collider& sphere, const Vec3f& direction);
		static Vec3f FurthestPointPlane(const Collider& plane, const Vec3f& direction);
		static Vec3f FurthestPointRect(const Collider& rect, const Vec3f& direction, Vec3f(&points)[8]);
		static Vec3f FurthestPointRect(const Collider& rect, const Vec3f& direction);
		static Vec3f FurthestPointCapsule(const Collider& capsule, const Vec3f& direction);
		static Vec3f FurthestPointHull(const Collider& hull, const Vec3f& direction);
		static Vec3f FurthestPointMesh(const Collider& mesh, const Vec3f& direction);

		static Vec3f MinkowskiFurthestPoint(const Collider& collider0, const Collider& collider1, const Vec3f direction);
		static Vec3f MinkowskiFurthestPointRect(const Collider& collider0, const Collider& rect1, const Vec3f direction, Vec3f(&points)[8]);

		static bool GJK(const Collider& collider0, const Collider& collider1, Simplex& outSimplex);
		static bool GJKRect(const Collider& collider0, const Collider& rect1, Vec3f(&points)[8], Simplex& outSimplex);

		static Collision EPA(const Collider& collider0, const Collider& collider1, const Simplex& simplex);

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

	public:
		static Vec3f FurthestPoint(const Collider& collider0, const Vec3f& direction);
		static Collision Collide(const Collider& collider0, const Collider& collider1);

		void Step(EntityWorld& world, double deltaTime);
		void Resolve(EntityWorld& world, double deltaTime);
	};
}