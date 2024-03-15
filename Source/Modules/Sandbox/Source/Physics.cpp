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

			Vec3f accel = (rigidBody.gravity * stepTime) + rigidBody.force * rigidBody.invMass;

			rigidBody.linearVelocity += accel;
			transform.position += rigidBody.linearVelocity * stepTime;

			physics.collider.transform = transform; /// ?

			rigidBody.force = Vec3f::ZERO;
		}
	}

	float CalcResolvedForces(const RigidBody& body0, const RigidBody& body1, const Vec3f& normal)
	{
		Vec3f relVelocity = body1.linearVelocity - body0.linearVelocity;
		float restitution = Min(body0.restitution, body1.restitution);
		float totalInvMass = body0.invMass + body1.invMass;

		if (Dot(relVelocity, normal) > 0.0f) // Moving toward each other
		{
			return (-(1.0f + restitution) * Dot(relVelocity, normal)) / (totalInvMass * Dot(normal, normal));
		}
		else // Moving away from each other, no force needed
		{
			return 0.0f;
		}
	}

	void Physics::ResolveCollisions(EntityWorld& world, RigidBodyView& rigidBodies, double stepTime)
	{
		auto& rigidBodiesForwardIt = rigidBodies.begin();

		Entity entity0;
		Entity entity1;

		for (auto& fit = rigidBodies.begin(); fit != rigidBodies.end(); ++fit)
		{
			entity0 = *fit;

			for (auto& rit = rigidBodies.rbegin(); rit != rigidBodies.rend(); --rit)
			{
				entity1 = *rit;

				if (entity0 == entity1)
				{
					break; // Ignore duplicates
				}

				RigidBodyComponent& physics0 = world.Get<RigidBodyComponent>(entity0);
				RigidBodyComponent& physics1 = world.Get<RigidBodyComponent>(entity1);
				TransformComponent& transform0 = world.Get<TransformComponent>(entity0);
				TransformComponent& transform1 = world.Get<TransformComponent>(entity1);

				RigidBody& rigidBody0 = physics0.rigidBody;
				RigidBody& rigidBody1 = physics1.rigidBody;

				Collider& collider0 = physics0.collider;
				Collider& collider1 = physics1.collider;

				if (collider0.isStatic && collider1.isStatic)
				{
					continue; // Ignore static-static collisions
				}

				Collision collision = Collide(collider0, collider1);
				Vec3f normal = (collision.extent1 - collision.extent0);

				if (collision.isColliding)
				{
					float j = CalcResolvedForces(rigidBody0, rigidBody1, normal);
					Vec3f impulse = j * normal;

					/*
					if (physics0.collider.isStatic)
					{
						transform1.position -= normal;
						rigidBody1.linearVelocity += impulse * rigidBody1.invMass;
					}
					else if (physics1.collider.isStatic)
					{
						transform0.position += normal;
						rigidBody0.linearVelocity -= impulse * rigidBody0.invMass;
					}
					else
					{
						transform0.position += normal / 2.0f;
						transform1.position -= normal / 2.0f;
						rigidBody0.linearVelocity -= impulse * rigidBody0.invMass;
						rigidBody1.linearVelocity += impulse * rigidBody1.invMass;
					}
					*/

					if (!physics0.collider.isStatic)
					{
						transform0.position += normal / 2.0f;
						rigidBody0.linearVelocity += -impulse * rigidBody0.invMass;
					}

					if (!physics1.collider.isStatic)
					{
						transform1.position += -normal / 2.0f;
						rigidBody1.linearVelocity += impulse * rigidBody1.invMass;
					}
				}

			}
		}
	}

	void Physics::ApplyImpulses(EntityWorld& world, RigidBodyView& rigidBodies, double stepTime)
	{

	}

	void Physics::Step(EntityWorld& world, double deltaTime)
	{
		RigidBodyView& rigidBodies = world.CreateView<RigidBodyComponent, TransformComponent>();

		for (uSize i = 0; i < PHYSICS_STEP_ITERATIONS; i++)
		{
			double stepTime = deltaTime / (double)PHYSICS_STEP_ITERATIONS;

			ApplyForces(world, rigidBodies, stepTime);
			ResolveCollisions(world, rigidBodies, stepTime);
			ApplyImpulses(world, rigidBodies, stepTime);
		}

	}
}

