#pragma once

#include "Engine.h"
#include "Colliders.h"
#include "Component/PhysicsComponent.h"
#include "Component/TransformComponent.h"

namespace Quartz
{
	class Physics
	{
	private:
		Collision ResolveCollision(Collider collider0, Collider collider1);

	public:
		void Step(EntityWorld& world, double deltaTime);
		void Resolve(EntityWorld& world, double deltaTime);
	};
}