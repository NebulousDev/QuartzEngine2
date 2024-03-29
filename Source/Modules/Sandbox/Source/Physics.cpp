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

			Vec3f linearAccel;
			Vec3f angularAccel;

			if (rigidBody.invMass)
			{
				linearAccel = rigidBody.gravity + rigidBody.force * (1.0f / rigidBody.invMass);
				angularAccel = rigidBody.torque * (1.0f / rigidBody.invMass);
			}

			/* Linear Integration */

			rigidBody.linearVelocity += linearAccel * stepTime;
			transform.position += rigidBody.linearVelocity * stepTime;
			rigidBody.force = Vec3f::ZERO;

			/* Angular Integration */

			rigidBody.angularVelocity += angularAccel * stepTime;
			transform.rotation *= rigidBody.angularVelocity * stepTime;
			transform.rotation.Normalize();
			rigidBody.torque = Vec3f::ZERO;

			//????
			rigidBody.lastAcceleration = linearAccel + angularAccel;
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

	inline Vec3f CalculateImpulse(const Contact& contact, const RigidBody& rigidBody0, const RigidBody& rigidBody1, const Vec3f& contactNormal)
	{
		const Vec3f torque0 = Cross(contact.localPoint0, contactNormal);
		const Vec3f angularMomentum0 = rigidBody0.invInertiaTensor * torque0;
		const Vec3f deltaVelocity0 = Cross(angularMomentum0, contact.localPoint0);

		float totalVelocity = 0;

		const float angularImpulse0 = Dot(deltaVelocity0, contactNormal);
		totalVelocity += angularImpulse0 + rigidBody0.invMass;

		if (rigidBody1.invMass != 0.0f)
		{
			const Vec3f torque1 = Cross(contact.localPoint1, contactNormal);
			const Vec3f angularMomentum1 = rigidBody1.invInertiaTensor * torque1;
			const Vec3f deltaVelocity1 = Cross(angularMomentum1, contact.localPoint1);

			const float angularImpulse1 = Dot(deltaVelocity1, contactNormal);
			totalVelocity += angularImpulse1 + rigidBody1.invMass;
		}

		const Vec3f impulseContact = Vec3f(contact.targetVelocity / totalVelocity, 0.0f, 0.0f);
		const Vec3f impulse = contact.invContactBasis * impulseContact;

		return impulse;
	}

	inline void BalanceMovements(float& inOutAngularMovement, float& inOutLinearMovement, float limit, float limitScale)
	{
		// Limit the rotational resolution to reduce new penetrations
		float scaledLimit = limit * limitScale;

		if (inOutAngularMovement < -scaledLimit)
		{
			float total = inOutLinearMovement + inOutAngularMovement;
			inOutAngularMovement = -scaledLimit;
			inOutLinearMovement = total - inOutAngularMovement;
		}
		else if (inOutAngularMovement > scaledLimit)
		{
			float total = inOutLinearMovement + inOutAngularMovement;
			inOutAngularMovement = scaledLimit;
			inOutLinearMovement = total - inOutAngularMovement;
		}
	}

	inline void CalculatePenetrationDeltas(Physics::CollisionData& collisionData, Contact& contact, 
		Vec3f& outDeltaLinear0, Vec3f& outDeltaLinear1, Vec3f& outDeltaAngular0, Vec3f& outDeltaAngular1)
	{
		const Collider& collider0 = collisionData.pRigidBody0->collider;
		const Collider& collider1 = collisionData.pRigidBody1->collider;
		const RigidBody& rigidBody0 = collisionData.pRigidBody0->rigidBody;
		const RigidBody& rigidBody1 = collisionData.pRigidBody1->rigidBody;

		float angularInertia0 = 0, angularInertia1 = 0;
		float linearInertia0 = 0, linearInertia1 = 0;
		float totalInertia = 0;

		{
			const Vec3f torque0 = Cross(contact.localPoint0, contact.normal);
			const Vec3f angularMomentum0 = rigidBody0.invInertiaTensor * torque0;
			const Vec3f deltaVelocity0 = Cross(angularMomentum0, contact.localPoint0);

			linearInertia0 = rigidBody0.invMass;
			angularInertia0 = Dot(deltaVelocity0, contact.normal);
			totalInertia += angularInertia0 + linearInertia0;

			if (rigidBody1.invMass != 0.0f)
			{
				const Vec3f torque1 = Cross(contact.localPoint1, contact.normal);
				const Vec3f angularMomentum1 = rigidBody1.invInertiaTensor * torque1;
				const Vec3f deltaVelocity1 = Cross(angularMomentum1, contact.localPoint1);

				linearInertia1 = rigidBody1.invMass;
				angularInertia1 = Dot(deltaVelocity1, contact.normal);
				totalInertia += angularInertia1 + linearInertia1;
			}
		}

		const float invInertia = 1.0f / totalInertia;
		const float balanceLimit = 0.2f;

		if (!collider0.IsStatic())
		{
			float linearMovement0 = contact.depth * linearInertia0 * invInertia;
			float angularMovement0 = contact.depth * angularInertia0 * invInertia;
			
			Vec3f proj = contact.localPoint0;
			proj += contact.normal * -Dot(contact.localPoint0, contact.normal);

			BalanceMovements(angularMovement0, linearMovement0, balanceLimit, proj.Magnitude());

			if (angularMovement0 != 0.0f)
			{
				const Vec3f angularVelocityImpulse0 = rigidBody0.invInertiaTensor * Cross(contact.localPoint0, contact.normal);
				const Vec3f rotation0 = angularVelocityImpulse0 * (angularMovement0 / angularInertia0);
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
			float linearMovement1 = -contact.depth * linearInertia1 * invInertia;
			float angularMovement1 = -contact.depth * angularInertia1 * invInertia;

			Vec3f proj = contact.localPoint0;
			proj += contact.normal * -Dot(contact.localPoint0, contact.normal);

			BalanceMovements(angularMovement1, linearMovement1, balanceLimit, proj.Magnitude());

			if (angularMovement1 != 0.0f)
			{
				const Vec3f angularVelocityImpulse1 = rigidBody1.invInertiaTensor * Cross(contact.localPoint1, contact.normal);
				const Vec3f rotation1 = angularVelocityImpulse1 * (angularMovement1 / angularInertia1);
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
		Vec3f& outDeltaLinearVel0, Vec3f& outDeltaLinearVel1, Vec3f& outDeltaAngularVel0, Vec3f& outDeltaAngularVel1)
	{
		const Collider&  collider0	= collisionData.pRigidBody0->collider;
		const Collider&  collider1	= collisionData.pRigidBody1->collider;
		const RigidBody& rigidBody0	= collisionData.pRigidBody0->rigidBody;
		const RigidBody& rigidBody1	= collisionData.pRigidBody1->rigidBody;

		/* Resolve Linear and Angular Velocities */

		Vec3f impulse = CalculateImpulse(contact, rigidBody0, rigidBody1, contact.normal);

		if (!collider0.IsStatic())
		{
			const Vec3f linearVelocity0 = impulse * rigidBody0.invMass;
			outDeltaLinearVel0 = linearVelocity0;

			const Vec3f impulsiveTorque0 = Cross(contact.localPoint0, impulse);
			const Vec3f angularVelocity0 = rigidBody0.invInertiaTensor * impulsiveTorque0;
			outDeltaAngularVel0 = angularVelocity0;
		}

		impulse *= -1;

		if (!collider1.IsStatic() && rigidBody1.invMass != 0.0f)
		{
			const Vec3f linearVelocity1 = impulse * -rigidBody1.invMass; // negative invMass?
			outDeltaLinearVel1 = linearVelocity1;

			const Vec3f impulsiveTorque1 = Cross(impulse, contact.localPoint1); // @Note: flipped from body0
			const Vec3f angularVelocity1 = rigidBody1.invInertiaTensor * impulsiveTorque1;
			outDeltaAngularVel1 = angularVelocity1;
		}
	}

	uSize FindNextDeepest(Collision& collision)
	{
		float maxDist = 0.0001f;
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
		float maxVel = 0.01f;
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
			while (posIteration++ < 10)
			{
				const uSize nextIndex = FindNextDeepest(collision);

				if (nextIndex == collision.count)
				{
					break;
				}

				Contact& contact = collision.contacts[nextIndex];

				// Calculate deltas

				Vec3f deltaAngular0, deltaAngular1;
				Vec3f deltaLinear0, deltaLinear1;

				CalculatePenetrationDeltas(collisionData, contact, 
					deltaLinear0, deltaLinear1, deltaAngular0, deltaAngular1);

				// Apply deltas

				transform0.Move(deltaLinear0);
				transform1.Move(deltaLinear1);
				transform0.Rotate(deltaAngular0);
				transform1.Rotate(deltaAngular1);

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

					Vec3f deltaPos0 = deltaLinear0 + Cross(deltaAngular0, contact.localPoint0);
					Vec3f deltaPos1 = deltaLinear1 + Cross(deltaAngular1, contact.localPoint1);

					nextContact.depth -= Dot(deltaPos0, nextContact.normal);
					nextContact.depth += Dot(deltaPos1, nextContact.normal);
				}
			}

			// Resolve velocities

			uSize velIteration = 0;
			while (velIteration++ < 10)
			{
				const uSize nextIndex = FindNextFastest(collision);

				if (nextIndex == collision.count)
				{
					break;
				}

				Contact& contact = collision.contacts[nextIndex];

				// Calculate deltas

				Vec3f deltaLinearVel0, deltaLinearVel1;
				Vec3f deltaAngularVel0, deltaAngularVel1;

				CalculateVelocityDeltas(collisionData, contact, 
					deltaLinearVel0, deltaLinearVel1, deltaAngularVel0, deltaAngularVel1);

				// Apply deltas

				//rigidBody0.AddLinearVelocity(deltaLinearVel0);
				//rigidBody1.AddLinearVelocity(deltaLinearVel1);
				//rigidBody0.AddAngularVelocity(deltaAngularVel0);
				//rigidBody1.AddAngularVelocity(deltaAngularVel1);

				// Adjust remaining contacts

				for (uSize j = 0; j < collision.count; j++) // @TODO: speed up
				{
					Contact& nextContact = collision.contacts[j];

					// Adjust velocities

					Vec3f deltaVel0 = deltaLinearVel0 + Cross(deltaAngularVel0, contact.localPoint0);
					Vec3f deltaVel1 = deltaLinearVel1 + Cross(deltaAngularVel1, contact.localPoint1);
					Vec3f deltaContactVel0 = nextContact.invContactBasis.Transposed() * deltaVel0;
					Vec3f deltaContactVel1 = nextContact.invContactBasis.Transposed() * deltaVel1;

					nextContact.contactVelocity += deltaContactVel0;
					nextContact.contactVelocity -= deltaContactVel1;

					nextContact.CalcTargetVelocity(rigidBody0.lastAcceleration, rigidBody1.lastAcceleration, 
						rigidBody0.restitution, rigidBody1.restitution, stepTime);
				}

				// Update inertia tensors

				//rigidBody0.UpdateInertia(transform0);
				//rigidBody1.UpdateInertia(transform1);
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

