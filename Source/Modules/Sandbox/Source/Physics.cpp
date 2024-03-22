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

			//????
			rigidBody.lastAcceleration = {};//linearAccel + angularAccel;
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


				// @TODO: BAD
				for (const CollisionData& data : (const Array<CollisionData>&)mCollisions)
				{
					if ((data.entity0 == entity0 && data.entity1 == entity1) || 
						(data.entity0 == entity1 && data.entity1 == entity0))
					{
						continue;
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
					collision.RecalcContactBasis();

					CollisionData data = { entity0, entity1, &physics0, &physics1, &transform0, &transform1, collision };

					mCollisions.PushBack(data);
				}
			}
		}
	}

	inline Vec3f CalculateImpulse(const Collision& collision, const RigidBody& rigidBody0, const RigidBody& rigidBody1, const Vec3f& contactNormal,
		Vec3f contactPointLocal0, Vec3f contactPointLocal1, const Transform& transform0, const Transform& transform1, double stepTime, 
		float& outAngularComponent0, float& outAngularComponent1)
	{
		float angularComponent = 0;
		Vec3f totalContactVelocity;

		if (rigidBody0.invMass != 0.0f)
		{
			Vec3f angularVelocityImpulse0 = rigidBody0.invInertiaTensor * Cross(contactPointLocal0, contactNormal);		// Angular contribution 
			Vec3f velocityImpulse0 = Cross(angularVelocityImpulse0, contactPointLocal0);								// Linear contribution

			outAngularComponent0 = Dot(velocityImpulse0, contactNormal);
			angularComponent += outAngularComponent0 + rigidBody0.invMass;

			Vec3f contactVelocity0 = Cross(rigidBody0.angularVelocity, contactPointLocal0) + rigidBody0.linearVelocity;	// Current velocity at the contact point
			totalContactVelocity += contactVelocity0;
		}

		if (rigidBody1.invMass != 0.0f)
		{
			Vec3f angularVelocityImpulse1 = rigidBody1.invInertiaTensor * Cross(contactPointLocal1, contactNormal);		// Angular contribution 
			Vec3f velocityImpulse1 = Cross(angularVelocityImpulse1, contactPointLocal1);								// Linear contribution

			outAngularComponent1 = Dot(velocityImpulse1, contactNormal);
			angularComponent += outAngularComponent1 + rigidBody1.invMass;

			Vec3f contactVelocity1 = Cross(rigidBody1.angularVelocity, contactPointLocal1) + rigidBody1.linearVelocity;	// Current velocity at the contact point
			totalContactVelocity += contactVelocity1;
		}

		Vec3f totalContactVelocityContact = collision.invContactBasis.Transposed() * totalContactVelocity;
		float restitution = Min(rigidBody0.restitution, rigidBody1.restitution);
		float desiredAngularComponent = -totalContactVelocityContact.x * (1.0f + restitution);

		Vec3f impulseContact = Vec3f(desiredAngularComponent / angularComponent, 0.0f, 0.0f);
		Vec3f impulse = collision.invContactBasis * impulseContact;

		return impulse;
	}

	void BalanceMovements(float& inOutAngularMovement, float& inOutLinearMovement, float limit, float limitScale)
	{
		// Limit the rotational resolution to reduce new penetrations
		float scaledLimit = limit * limitScale;
		if (Abs(inOutAngularMovement) > scaledLimit)
		{
			float total = inOutLinearMovement + inOutAngularMovement;

			if (inOutAngularMovement >= 0.0f)
			{
				inOutAngularMovement = scaledLimit;
			}
			else
			{
				inOutAngularMovement = -scaledLimit;
			}

			// Linear resolution picks up the slack
			inOutLinearMovement = total - inOutAngularMovement;
		}
	}

	void Physics::ResolveCollisions(EntityWorld& world, RigidBodyView& rigidBodies, double stepTime)
	{
		for (CollisionData& collisionData : mCollisions)
		{
			Collider&  collider0	= collisionData.pRigidBody0->collider;
			Collider&  collider1	= collisionData.pRigidBody1->collider;
			RigidBody& rigidBody0	= collisionData.pRigidBody0->rigidBody;
			RigidBody& rigidBody1	= collisionData.pRigidBody1->rigidBody;
			Transform& transform0	= *collisionData.pTransform0;
			Transform& transform1	= *collisionData.pTransform1;

			Collision& collision = collisionData.collision;

			Vec3f normal = collision.normal.Normalized();
			float depth = collision.depth;

			// Contact points relative to the object's center
			Vec3f contactPointLocal0 = collision.points[0] - transform0.position;
			Vec3f contactPointLocal1 = collision.points[0] - transform1.position;

			/* Resolve Linear and Angular Velocities */

			float angularInertia0 = 0;
			float angularInertia1 = 0;
			float linerInertia0 = 0;
			float linerInertia1 = 0;

			float totalInertia = 0;

			Vec3f impulse = CalculateImpulse(collision, rigidBody0, rigidBody1, 
				normal, contactPointLocal0, contactPointLocal1, transform0, transform1, stepTime,
				angularInertia0, angularInertia1);

			if (!collider0.IsStatic())
			{
				Vec3f impulsiveTorque0 = Cross(impulse, contactPointLocal0); // flipped?
				Vec3f finalAngularVelocity0 = rigidBody0.invInertiaTensor * impulsiveTorque0;
				Vec3f finalLinearVelocity0 = impulse * rigidBody0.invMass;

				rigidBody0.AddLinearVelocity(finalLinearVelocity0);
				rigidBody0.AddAngularVelocity(finalAngularVelocity0);

				linerInertia0 = rigidBody0.invMass;
				totalInertia += linerInertia0 + angularInertia0;
			}

			impulse *= -1;

			if (!collider1.IsStatic())
			{
				Vec3f impulsiveTorque1 = Cross(impulse, contactPointLocal1); // flipped?
				Vec3f finalAngularVelocity1 = rigidBody1.invInertiaTensor * impulsiveTorque1;
				Vec3f finalLinearVelocity1 = impulse * rigidBody1.invMass;

				rigidBody1.AddLinearVelocity(finalLinearVelocity1);
				rigidBody1.AddAngularVelocity(finalAngularVelocity1);

				linerInertia1 = rigidBody1.invMass;
				totalInertia += linerInertia1 + angularInertia1;
			}

			/* Resolve Penetration */

			float invInertia = 1.0f / totalInertia;
			float balanceLimit = 0.2f;

			if (!collider0.IsStatic())
			{
				//angularInertia0 += rigidBody0.invMass;

				float linearMovement0 = depth * linerInertia0 * invInertia;
				float angularMovement0 = depth * angularInertia0 * invInertia;
				
				BalanceMovements(angularMovement0, linearMovement0, balanceLimit, contactPointLocal0.Magnitude());

				if (angularInertia0 != 0.0f)
				{
					Vec3f angularVelocityImpulse0 = rigidBody0.invInertiaTensor * Cross(contactPointLocal0, normal); // Get from CalcImpulse
					Vec3f rotation0 = angularVelocityImpulse0 * (1.0f / angularInertia0) * angularMovement0;
					transform0.Rotate(rotation0);
				}

				transform0.Move(normal * linearMovement0);
			}

			if (!collider1.IsStatic())
			{
				//angularInertia1 += rigidBody0.invMass;

				float linearMovement1 = -depth * linerInertia1 * invInertia;
				float angularMovement1 = -depth * angularInertia1 * invInertia;

				BalanceMovements(angularMovement1, linearMovement1, balanceLimit, contactPointLocal1.Magnitude());

				if (angularInertia1 != 0.0f)
				{
					Vec3f angularVelocityImpulse1 = rigidBody1.invInertiaTensor * Cross(contactPointLocal1, normal);
					Vec3f rotation1 = angularVelocityImpulse1 * (1.0f / angularInertia1) * angularMovement1;
					transform1.Rotate(rotation1);
				}

				transform1.Move(normal * linearMovement1);
			}
		}
	}

	void Physics::OnRigidBodyAdded(Runtime& runtime, const ComponentAddedEvent<RigidBodyComponent>& event)
	{
		RigidBody& rigidBody	= event.component.rigidBody;
		Collider& collider		= event.component.collider;
		Transform& transform	= event.world.Get<TransformComponent>(event.entity);

		Vec3f initInertia = InitalInertia(rigidBody, collider, transform.scale);

		if (!initInertia.IsZero())
		{
			// @TODO: make sure this gets updated
			rigidBody.invInertiaTensor = Mat3f().SetIdentity(InitalInertia(rigidBody, collider, transform.scale)).Inverse();
		}

		//rigidBody.invInertiaTensor = Mat3f().SetIdentity();
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

			// @TODO separate collisions and contact calculations
			FindCollisions(world, rigidBodies, stepTime);
			ResolveCollisions(world, rigidBodies, stepTime);
			ApplyForces(world, rigidBodies, stepTime);
		}

	}
}

