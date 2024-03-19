#include "Physics.h"

namespace Quartz
{
	void Physics::ApplyForces(EntityWorld& world, RigidBodyView& rigidBodies, double stepTime)
	{
		for (Entity& entity : rigidBodies)
		{
			RigidBodyComponent& physics = world.Get<RigidBodyComponent>(entity);
			TransformComponent& transform = world.Get<TransformComponent>(entity);

			RigidBody& rigidBody = physics.rigidBody;

			if (rigidBody.asleep)
			{
				continue;
			}

			/* Linear Integration */

			Vec3f linearAccel = (rigidBody.gravity * stepTime) + rigidBody.force * rigidBody.invMass;
			rigidBody.linearVelocity += linearAccel;

			transform.position += rigidBody.linearVelocity * stepTime;

			rigidBody.force = Vec3f::ZERO;

			/* Angular Integration */

			Mat3f mrot = Mat3f().SetRotation(transform.rotation);
			Vec3f angularAccel = mrot * rigidBody.invInertiaTensor * mrot.Transposed() * rigidBody.torque;// *stepTime;
			rigidBody.angularVelocity += angularAccel;

			transform.rotation *= rigidBody.angularVelocity * stepTime;
			transform.rotation.Normalize();

			rigidBody.torque = Vec3f::ZERO;
		}
	}

	float CalcResolvedForces(const RigidBody& body0, const RigidBody& body1, const Vec3f& normal)
	{
		Vec3f relVelocity = body1.linearVelocity - body0.linearVelocity;
		float restitution = Min(body0.restitution, body1.restitution);
		float totalInvMass = body0.invMass + body1.invMass;

		if (Dot(relVelocity, normal) < 0.0f) // Moving toward each other
		{
			return (-(1.0f + restitution) * Dot(relVelocity, normal)) / (totalInvMass * Dot(normal, normal));
		}
		else // Moving away from each other, no force needed
		{
			return 0.0f;
		}
	}

	void Physics::FindCollisions(EntityWorld& world, RigidBodyView& rigidBodies, double stepTime)
	{	
		Entity entity0;
		Entity entity1;

		mCollisions.Clear();

		for (auto& fit = rigidBodies.begin(); fit != rigidBodies.end(); ++fit)
		{
			entity0 = *fit;

			//for (auto& rit = rigidBodies.rbegin(); rit != rigidBodies.rend(); --rit)
			for (auto& rit = rigidBodies.begin(); rit != rigidBodies.end(); ++rit)
			{
				entity1 = *rit;

				if (entity0 == entity1)
				{
					//break; // Ignore duplicates
					continue;
				}

				RigidBodyComponent& physics0	= world.Get<RigidBodyComponent>(entity0);
				TransformComponent& transform0	= world.Get<TransformComponent>(entity0);
				RigidBody& rigidBody0			= physics0.rigidBody;
				Collider& collider0				= physics0.collider;

				RigidBodyComponent& physics1	= world.Get<RigidBodyComponent>(entity1);
				TransformComponent& transform1	= world.Get<TransformComponent>(entity1);
				RigidBody& rigidBody1			= physics1.rigidBody;
				Collider& collider1				= physics1.collider;

				collider0.transform = transform0;
				collider1.transform = transform1;

				if (collider0.isStatic && collider1.isStatic)
				{
					continue; // Ignore static-static collisions
				}

				Collision collision = Collide(collider0, collider1);
				
				if (collision.isColliding)
				{
					CollisionData data = { &physics0, &physics1, &transform0, &transform1, collision };
					mCollisions.PushBack(data);
				}
			}
		}
	}

	void Physics::ApplyImpulses(EntityWorld& world, RigidBodyView& rigidBodies, double stepTime)
	{
		for (CollisionData& collisionData : mCollisions)
		{
			RigidBody& rigidBody0	= collisionData.pRigidBody0->rigidBody;
			Transform& transform0	= *collisionData.pTransform0;
			Collider&  collider0	= collisionData.pRigidBody0->collider;

			RigidBody& rigidBody1	= collisionData.pRigidBody1->rigidBody;
			Transform& transform1	= *collisionData.pTransform1;
			Collider&  collider1	= collisionData.pRigidBody1->collider;

			Collision& collision	= collisionData.collision;

			Vec3f normal = collision.normal;
			float dist = collision.dist;

			Vec3f scaledNormal = normal * dist;

			float j = CalcResolvedForces(rigidBody0, rigidBody1, scaledNormal);
			Vec3f linearImpulse = j * scaledNormal;

			if (!collider0.isStatic)
			{
				if (collider1.isStatic)
				{
					transform0.position += -scaledNormal;
				}
				else
				{
					transform0.position += -scaledNormal / 2.0f;
				}

				rigidBody0.AddLinearVelocity(-linearImpulse * rigidBody0.invMass);
			}

			if (!collider1.isStatic)
			{
				if (collider0.isStatic)
				{
					transform1.position += scaledNormal;
				}
				else
				{
					transform1.position += scaledNormal / 2.0f;
				}

				rigidBody1.AddLinearVelocity(linearImpulse * rigidBody1.invMass);
			}
		}
	}

	void Physics::OnRigidBodyAdded(Runtime& runtime, const ComponentAddedEvent<RigidBodyComponent>& event)
	{
		RigidBody& rigidBody	= event.component.rigidBody;
		Collider& collider		= event.component.collider;

		Vec3f initInertia = InitalInertia(rigidBody, collider);

		if (!initInertia.IsZero())
		{
			rigidBody.invInertiaTensor = Mat3f().SetIdentity(InitalInertia(rigidBody, collider)).Inverse();
		}
	}

	void Physics::Initialize()
	{
		Engine::GetRuntime().RegisterOnEvent<ComponentAddedEvent<RigidBodyComponent>>(&Physics::OnRigidBodyAdded, this);
	}

	void Physics::Step(EntityWorld& world, double deltaTime)
	{
		RigidBodyView& rigidBodies = world.CreateView<RigidBodyComponent, TransformComponent>();

		for (uSize i = 0; i < PHYSICS_STEP_ITERATIONS; i++)
		{
			double stepTime = deltaTime / (double)PHYSICS_STEP_ITERATIONS;

			ApplyForces(world, rigidBodies, stepTime);
			ApplyImpulses(world, rigidBodies, stepTime);
			FindCollisions(world, rigidBodies, stepTime);
		}

	}
}

