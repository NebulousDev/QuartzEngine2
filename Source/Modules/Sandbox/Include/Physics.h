#pragma once

#include "Engine.h"
#include "Colliders.h"
#include "CollisionDetection.h"
#include "Entity/World.h"
#include "Component/PhysicsComponent.h"
#include "Component/TransformComponent.h"

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
			Entity entity0;
			Entity entity1;
			RigidBodyComponent* pRigidBody0;
			RigidBodyComponent* pRigidBody1;
			TransformComponent* pTransform0;
			TransformComponent* pTransform1;

			Collision collision;
		};

	private:
		static CollisionDetection collisionDetection;

		Array<CollisionData> mCollisions;

	private:

		/* Default Inertia */

		static Vec3f InitalInertiaSphere(const RigidBody& rigidBody, const SphereCollider& sphere, const Vec3f& scale);
		static Vec3f InitalInertiaPlane(const RigidBody& rigidBody, const PlaneCollider& plane, const Vec3f& scale);
		static Vec3f InitalInertiaRect(const RigidBody& rigidBody, const RectCollider& rect, const Vec3f& scale);
		static Vec3f InitalInertiaCapsule(const RigidBody& rigidBody, const CapsuleCollider& capsule, const Vec3f& scale);
		static Vec3f InitalInertiaHull(const RigidBody& rigidBody, const HullCollider& hull, const Vec3f& scale);
		static Vec3f InitalInertiaMesh(const RigidBody& rigidBody, const MeshCollider& mesh, const Vec3f& scale);

		static Vec3f InitalInertia(const RigidBody& rigidBody, const Collider& collider, const Vec3f& scale);

		/* Apply Physics */

		void ApplyForces(EntityWorld& world, RigidBodyView& rigidBodies, double stepTime);
		void FindCollisions(EntityWorld& world, RigidBodyView& rigidBodies, double stepTime);
		void ResolveCollisions(EntityWorld& world, RigidBodyView& rigidBodies, double stepTime);

		/* Triggers */

		void OnRigidBodyAdded(Runtime& runtime, const ComponentAddedEvent<RigidBodyComponent>& event);

	public:
		void Initialize();

		bool Collide(const Collider& collider0, const Transform& transform0,
			const Collider& collider1, const Transform& transform1, Collision& outCollision);

		//void GenerateContacts(const Collider& collider0, const Collider& collider1, const Collision& collision);

		void Step(EntityWorld& world, double deltaTime);
	};
}