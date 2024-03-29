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

			Vec3p linearAccel;
			Vec3p angularAccel;

			if (rigidBody.invMass)
			{
				linearAccel = rigidBody.gravity + rigidBody.force * (1.0f / rigidBody.invMass);
				angularAccel = rigidBody.torque * (1.0f / rigidBody.invMass);
			}

			/* Linear Integration */

			rigidBody.linearVelocity += linearAccel * stepTime;
			transform.position += rigidBody.linearVelocity * stepTime;
			rigidBody.force = Vec3p::ZERO;

			/* Angular Integration */

			rigidBody.angularVelocity += angularAccel * stepTime;
			transform.rotation *= rigidBody.angularVelocity * stepTime;
			transform.rotation.Normalize();
			rigidBody.torque = Vec3p::ZERO;

			//????
			rigidBody.lastAcceleration = rigidBody.gravity * rigidBody.invMass; //linearAccel + angularAccel;
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


				// @TODO: BAD
				for (const CollisionData& data : (const Array<CollisionData>&)mCollisions)
				{
					if ((data.entity0 == entity0 && data.entity1 == entity1) || 
						(data.entity0 == entity1 && data.entity1 == entity0))
					{
						goto endLoop;
					}
				}

				if (collider0.IsStatic() && collider1.IsStatic())
				{
					continue; // Ignore static-static collisions
				}

				Collision collision; 
				bool colliding = Collide(collider0, transform0, collider1, transform1, collision);
				
				if (colliding)
				{
					if (rigidBody0.invMass == 0.0f) // Ensure the first object has mass
					{
						collision.Flip();
						CollisionData data = { entity1, entity0, &physics1, &physics0, &transform1, &transform0, collision };
						mCollisions.PushBack(data);
					}
					else
					{
						CollisionData data = { entity0, entity1, &physics0, &physics1, &transform0, &transform1, collision };
						mCollisions.PushBack(data);
					}
				}

			endLoop:
				continue;
			}
		}
	}

	inline Vec3p CalculateImpulse(const Contact& contact, const RigidBody& rigidBody0, const RigidBody& rigidBody1)
	{
		const Vec3p torque0 = Cross(contact.localPoint0, contact.normal);
		const Vec3p angularMomentum0 = rigidBody0.invInertiaTensor * torque0;
		const Vec3p deltaVelocity0 = Cross(angularMomentum0, contact.localPoint0);

		floatp totalVelocity = 0;

		const floatp angularImpulse0 = Dot(deltaVelocity0, contact.normal);
		totalVelocity += angularImpulse0 + rigidBody0.invMass;

		if (rigidBody1.invMass != 0.0f)
		{
			const Vec3p torque1 = Cross(contact.localPoint1, contact.normal);
			const Vec3p angularMomentum1 = rigidBody1.invInertiaTensor * torque1;
			const Vec3p deltaVelocity1 = Cross(angularMomentum1, contact.localPoint1);

			const floatp angularImpulse1 = Dot(deltaVelocity1, contact.normal);
			totalVelocity += angularImpulse1 + rigidBody1.invMass;
		}

		const Vec3p impulseContact = Vec3p(contact.targetVelocity / totalVelocity, 0.0f, 0.0f);
		const Vec3p impulse = contact.invContactBasis * impulseContact;

		return impulse;
	}

	inline void BalanceMovements(floatp& inOutAngularMovement, floatp& inOutLinearMovement, floatp limit, floatp limitScale)
	{
		// Limit the rotational resolution to reduce new penetrations
		floatp scaledLimit = limit * limitScale;

		if (inOutAngularMovement < -scaledLimit)
		{
			floatp total = inOutLinearMovement + inOutAngularMovement;
			inOutAngularMovement = -scaledLimit;
			inOutLinearMovement = total - inOutAngularMovement;
		}
		else if (inOutAngularMovement > scaledLimit)
		{
			floatp total = inOutLinearMovement + inOutAngularMovement;
			inOutAngularMovement = scaledLimit;
			inOutLinearMovement = total - inOutAngularMovement;
		}
	}

	inline void CalculatePenetrationDeltas(Physics::CollisionData& collisionData, Contact& contact, 
		Vec3p& outDeltaLinear0, Vec3p& outDeltaLinear1, Vec3p& outDeltaAngular0, Vec3p& outDeltaAngular1)
	{
		const Collider& collider0 = collisionData.pRigidBody0->collider;
		const Collider& collider1 = collisionData.pRigidBody1->collider;
		const RigidBody& rigidBody0 = collisionData.pRigidBody0->rigidBody;
		const RigidBody& rigidBody1 = collisionData.pRigidBody1->rigidBody;

		floatp angularInertia0 = 0, angularInertia1 = 0;
		floatp linearInertia0 = 0, linearInertia1 = 0;
		floatp totalInertia = 0;

		{
			const Vec3p torque0 = Cross(contact.localPoint0, contact.normal);
			const Vec3p angularMomentum0 = rigidBody0.invInertiaTensor * torque0;
			const Vec3p deltaVelocity0 = Cross(angularMomentum0, contact.localPoint0);

			linearInertia0 = rigidBody0.invMass;
			angularInertia0 = Dot(deltaVelocity0, contact.normal);
			totalInertia += angularInertia0 + linearInertia0;

			if (rigidBody1.invMass != 0.0f)
			{
				const Vec3p torque1 = Cross(contact.localPoint1, contact.normal);
				const Vec3p angularMomentum1 = rigidBody1.invInertiaTensor * torque1;
				const Vec3p deltaVelocity1 = Cross(angularMomentum1, contact.localPoint1);

				linearInertia1 = rigidBody1.invMass;
				angularInertia1 = Dot(deltaVelocity1, contact.normal);
				totalInertia += angularInertia1 + linearInertia1;
			}
		}

		const floatp invInertia = 1.0f / totalInertia;
		const floatp balanceLimit = 0.2f;

		if (!collider0.IsStatic())
		{
			floatp linearMovement0 = contact.depth * linearInertia0 * invInertia;
			floatp angularMovement0 = contact.depth * angularInertia0 * invInertia;
			
			Vec3p proj = contact.localPoint0;
			proj += contact.normal * -Dot(contact.localPoint0, contact.normal);

			BalanceMovements(angularMovement0, linearMovement0, balanceLimit, proj.Magnitude());

			if (angularMovement0 != 0.0f)
			{
				const Vec3p angularVelocityImpulse0 = rigidBody0.invInertiaTensor * Cross(contact.localPoint0, contact.normal);
				const Vec3p rotation0 = angularVelocityImpulse0 * (angularMovement0 / angularInertia0);
				outDeltaAngular0 = rotation0;
			}
			else
			{
				outDeltaAngular0 = 0;
			}

			outDeltaLinear0 = contact.normal * linearMovement0;
		}

		if (!collider1.IsStatic() && rigidBody1.invMass != 0.0f)
		{
			floatp linearMovement1 = -contact.depth * linearInertia1 * invInertia;
			floatp angularMovement1 = -contact.depth * angularInertia1 * invInertia;

			Vec3p proj = contact.localPoint0;
			proj += contact.normal * -Dot(contact.localPoint0, contact.normal);

			BalanceMovements(angularMovement1, linearMovement1, balanceLimit, proj.Magnitude());

			if (angularMovement1 != 0.0f)
			{
				const Vec3p angularVelocityImpulse1 = rigidBody1.invInertiaTensor * Cross(contact.localPoint1, contact.normal);
				const Vec3p rotation1 = angularVelocityImpulse1 * (angularMovement1 / angularInertia1);
				outDeltaAngular1 = rotation1;
			}
			else
			{
				outDeltaAngular1 = 0;
			}

			outDeltaLinear1 = contact.normal * linearMovement1;
		}
	}

	inline void CalculateVelocityDeltas(Physics::CollisionData& collisionData, Contact& contact,
		Vec3p& outDeltaLinearVel0, Vec3p& outDeltaLinearVel1, Vec3p& outDeltaAngularVel0, Vec3p& outDeltaAngularVel1)
	{
		const Collider&  collider0	= collisionData.pRigidBody0->collider;
		const Collider&  collider1	= collisionData.pRigidBody1->collider;
		const RigidBody& rigidBody0	= collisionData.pRigidBody0->rigidBody;
		const RigidBody& rigidBody1	= collisionData.pRigidBody1->rigidBody;

		/* Resolve Linear and Angular Velocities */

		Vec3p impulse = CalculateImpulse(contact, rigidBody0, rigidBody1);

		if (!collider0.IsStatic())
		{
			const Vec3p linearVelocity0 = impulse * rigidBody0.invMass;
			outDeltaLinearVel0 = linearVelocity0;

			const Vec3p impulsiveTorque0 = Cross(contact.localPoint0, impulse);
			const Vec3p angularVelocity0 = rigidBody0.invInertiaTensor * impulsiveTorque0;
			outDeltaAngularVel0 = angularVelocity0;
		}

		impulse *= -1;

		if (!collider1.IsStatic() && rigidBody1.invMass != 0.0f)
		{
			const Vec3p linearVelocity1 = impulse * -rigidBody1.invMass; // negative invMass?
			outDeltaLinearVel1 = linearVelocity1;

			const Vec3p impulsiveTorque1 = Cross(impulse, contact.localPoint1); // @Note: flipped from body0
			const Vec3p angularVelocity1 = rigidBody1.invInertiaTensor * impulsiveTorque1;
			outDeltaAngularVel1 = angularVelocity1;
		}
	}

	uSize FindNextDeepest(Collision& collision)
	{
		floatp maxDist = PHYSICS_SMALLEST_DISTANCE;
		uSize maxIndex = collision.count;

		for (uSize i = 0; i < collision.count; i++) // @TODO: speed up
		{
			Contact& contact = collision.contacts[i];

			if (contact.depth > maxDist)
			{
				maxDist = contact.depth;
				maxIndex = i;
			}
		}
		
		return maxIndex;
	}

	uSize FindNextFastest(Collision& collision)
	{
		floatp maxVel = PHYSICS_SMALLEST_VELOCITY;
		uSize maxIndex = collision.count;

		for (uSize i = 0; i < collision.count; i++) // @TODO: speed up
		{
			Contact& contact = collision.contacts[i];

			if (contact.targetVelocity > maxVel)
			{
				maxVel = contact.targetVelocity;
				maxIndex = i;
			}
		}

		return maxIndex;
	}

	void Physics::ResolveCollisions(EntityWorld& world, RigidBodyView& rigidBodies, double stepTime)
	{
		for (CollisionData& collisionData : mCollisions)
		{
			Collision& collision = collisionData.collision;

			RigidBody& rigidBody0 = collisionData.pRigidBody0->rigidBody;
			RigidBody& rigidBody1 = collisionData.pRigidBody1->rigidBody;
			Transform& transform0 = *collisionData.pTransform0;
			Transform& transform1 = *collisionData.pTransform1;

			// Calculate contact data

			for (uSize i = 0; i < collision.count; i++) // @TODO: speed up
			{
				Contact& contact = collision.contacts[i];

				contact.CalcContactBasis();
				contact.CalcLocalPoints(transform0.position, transform1.position);
				contact.CalcContactVelocity(rigidBody0.lastAcceleration, rigidBody1.lastAcceleration, 
					rigidBody0.angularVelocity, rigidBody1.angularVelocity,
					rigidBody0.linearVelocity, rigidBody1.linearVelocity, stepTime);
				contact.CalcTargetVelocity(rigidBody0.lastAcceleration, rigidBody1.lastAcceleration, 
					rigidBody0.restitution, rigidBody1.restitution, stepTime);
			}

			// Resolve penetrations

			uSize posIteration = 0;
			while (posIteration++ < PHYSICS_MAX_RESOLVE_ITERATIONS)
			{
				const uSize nextIndex = FindNextDeepest(collision);

				if (nextIndex == collision.count)
				{
					break;
				}

				Contact& contact = collision.contacts[nextIndex];

				// Calculate deltas

				Vec3p deltaAngular0, deltaAngular1;
				Vec3p deltaLinear0, deltaLinear1;

				CalculatePenetrationDeltas(collisionData, contact, 
					deltaLinear0, deltaLinear1, deltaAngular0, deltaAngular1);

				// Apply deltas

				transform0.Move(Vec3f(deltaLinear0));
				transform1.Move(Vec3f(deltaLinear1));
				transform0.Rotate(Vec3f(deltaAngular0));
				transform1.Rotate(Vec3f(deltaAngular1));

				transform0.rotation.Normalize();
				transform1.rotation.Normalize();

				// Update inertia tensors

				rigidBody0.UpdateInertia(transform0);
				rigidBody1.UpdateInertia(transform1);

				// Adjust remaining contacts

				for (uSize j = 0; j < collision.count; j++) // @TODO: speed up
				{
					Contact& nextContact = collision.contacts[j];

					// Adjust depths

					Vec3p deltaPos0 = deltaLinear0 + Cross(deltaAngular0, contact.localPoint0);
					Vec3p deltaPos1 = deltaLinear1 + Cross(deltaAngular1, contact.localPoint1);

					nextContact.depth -= Dot(deltaPos0, nextContact.normal);
					nextContact.depth += Dot(deltaPos1, nextContact.normal);
				}
			}

			// Resolve velocities

			uSize velIteration = 0;
			while (velIteration++ < PHYSICS_MAX_VELOCITY_ITERATIONS)
			{
				const uSize nextIndex = FindNextFastest(collision);

				if (nextIndex == collision.count)
				{
					break;
				}

				Contact& contact = collision.contacts[nextIndex];

				// Calculate deltas

				Vec3p deltaLinearVel0, deltaLinearVel1;
				Vec3p deltaAngularVel0, deltaAngularVel1;

				CalculateVelocityDeltas(collisionData, contact, 
					deltaLinearVel0, deltaLinearVel1, deltaAngularVel0, deltaAngularVel1);

				// Apply deltas

				rigidBody0.AddLinearVelocity(deltaLinearVel0);
				rigidBody1.AddLinearVelocity(deltaLinearVel1);
				//rigidBody0.AddAngularVelocity(deltaAngularVel0);
				//rigidBody1.AddAngularVelocity(deltaAngularVel1);

				// Adjust remaining contacts

				for (uSize j = 0; j < collision.count; j++) // @TODO: speed up
				{
					Contact& nextContact = collision.contacts[j];

					// Adjust velocities

					Vec3p deltaVel0 = deltaLinearVel0 + Cross(deltaAngularVel0, contact.localPoint0);
					Vec3p deltaVel1 = deltaLinearVel1 + Cross(deltaAngularVel1, contact.localPoint1);
					Vec3p deltaContactVel0 = nextContact.invContactBasis.Transposed() * deltaVel0;
					Vec3p deltaContactVel1 = nextContact.invContactBasis.Transposed() * deltaVel1;

					nextContact.contactVelocity += deltaContactVel0;
					nextContact.contactVelocity -= deltaContactVel1;

					nextContact.CalcTargetVelocity(rigidBody0.lastAcceleration, rigidBody1.lastAcceleration, 
						rigidBody0.restitution, rigidBody1.restitution, stepTime);
				}
			}
		}
	}

	void Physics::OnRigidBodyAdded(Runtime& runtime, const ComponentAddedEvent<RigidBodyComponent>& event)
	{
		RigidBody& rigidBody	= event.component.rigidBody;
		Collider& collider		= event.component.collider;
		Transform& transform	= event.world.Get<TransformComponent>(event.entity);

		rigidBody.inertiaVector = InitalInertia(rigidBody, collider, transform.scale);
		rigidBody.UpdateInertia(transform);
	}

	void Physics::Initialize()
	{
		Engine::GetRuntime().RegisterOnEvent<ComponentAddedEvent<RigidBodyComponent>>(&Physics::OnRigidBodyAdded, this);
	}

	bool Physics::Collide(const Collider& collider0, const Transform& transform0, 
		const Collider& collider1, const Transform& transform1, Collision& outCollision)
	{
		return collisionDetection.Collide(collider0, transform0, collider1, transform1, outCollision);
	}

	void Physics::Step(EntityWorld& world, double deltaTime)
	{
		RigidBodyView& rigidBodies = world.CreateView<RigidBodyComponent, TransformComponent>();

		for (uSize i = 0; i < PHYSICS_STEP_ITERATIONS; i++)
		{
			double stepTime = deltaTime / (double)PHYSICS_STEP_ITERATIONS;

			FindCollisions(world, rigidBodies, stepTime);
			ResolveCollisions(world, rigidBodies, stepTime);
			ApplyForces(world, rigidBodies, stepTime);
		}

	}
}

