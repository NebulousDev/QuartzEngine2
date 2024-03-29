#pragma once

#include "Engine.h"
#include "Colliders.h"
#include "CollisionDetection.h"
#include "Entity/World.h"
#include "PhysicsTypes.h"
#include "Component/PhysicsComponent.h"
#include "Component/TransformComponent.h"

#define PHYSICS_STEP_ITERATIONS			8
#define PHYSICS_MAX_RESOLVE_ITERATIONS	8
#define PHYSICS_MAX_VELOCITY_ITERATIONS	8
#define PHYSICS_SMALLEST_DISTANCE		0.001f
#define PHYSICS_SMALLEST_VELOCITY		0.200f

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

		static Vec3p InitalInertiaSphere(const RigidBody& rigidBody, const SphereCollider& sphere, const Vec3p& scale);
		static Vec3p InitalInertiaPlane(const RigidBody& rigidBody, const PlaneCollider& plane, const Vec3p& scale);
		static Vec3p InitalInertiaRect(const RigidBody& rigidBody, const RectCollider& rect, const Vec3p& scale);
		static Vec3p InitalInertiaCapsule(const RigidBody& rigidBody, const CapsuleCollider& capsule, const Vec3p& scale);
		static Vec3p InitalInertiaHull(const RigidBody& rigidBody, const HullCollider& hull, const Vec3p& scale);
		static Vec3p InitalInertiaMesh(const RigidBody& rigidBody, const MeshCollider& mesh, const Vec3p& scale);

		static Vec3p InitalInertia(const RigidBody& rigidBody, const Collider& collider, const Vec3p& scale);

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