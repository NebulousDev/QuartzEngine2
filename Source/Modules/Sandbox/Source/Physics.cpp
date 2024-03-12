#include "Physics.h"

namespace Quartz
{
	void Physics::Step(EntityWorld& world, double deltaTime)
	{
		auto& renderableView = world.CreateView<RigidBodyComponent, TransformComponent>();

		for (Entity& entity : renderableView)
		{
			RigidBodyComponent& physics = world.Get<RigidBodyComponent>(entity);
			TransformComponent& transform = world.Get<TransformComponent>(entity);

			//physics.force += (-physics.force * physics.friction);

			Vec3f accel = physics.force / physics.mass;
						
			physics.velocity	+= accel * deltaTime;
			transform.position	+= physics.velocity * deltaTime;

			physics.collider.transform = transform; /// ?
		}
	}

	void Physics::Resolve(EntityWorld& world, double deltaTime)
	{
		auto& renderableView = world.CreateView<RigidBodyComponent, TransformComponent>();

		for (Entity& entity0 : renderableView)
		{
			for (Entity& entity1 : renderableView)
			{
				if (entity0 == entity1)
				{
					break; // Ignore duplicates
				}

				RigidBodyComponent& physics0 = world.Get<RigidBodyComponent>(entity0);
				RigidBodyComponent& physics1 = world.Get<RigidBodyComponent>(entity1);
				TransformComponent& transform0 = world.Get<TransformComponent>(entity0);
				TransformComponent& transform1 = world.Get<TransformComponent>(entity1);

				Collision collision = ResolveCollision(physics0.collider, physics1.collider);

				if (collision.isColliding)
				{
					if (!physics0.collider.isStatic)
					{
						transform0.position += (collision.extent1 - collision.extent0) / 2.0f;
					}

					if (!physics1.collider.isStatic)
					{
						transform1.position -= (collision.extent1 - collision.extent0) / 2.0f;
					}

					//physics1.force = {};
				}

			}
		}
	}
}

