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

	void ResolveCollisionDepths(const Collider& collider0, const Collider& collider1, const Vec3f scaledNormal, Transform& transform0, Transform& transform1)
	{
		if (!collider0.isStatic)
		{
			if (collider1.isStatic)
			{
				transform0.position -= scaledNormal;
			}
			else
			{
				transform0.position -= scaledNormal / 2.0f;
			}
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
		}
	}

	Vec3f CalcContactVelocity(const Collision& collision, const RigidBody& rigidBody0, 
		const RigidBody& rigidBody1, const Transform& transform0, const Transform& transform1, double stepTime)
	{
		/* Body 0 */

		Vec3f contactPointLocal0 = collision.contact0[0] - transform0.position;
		Vec3f totalContactVelocityLocal0 = Cross(rigidBody0.angularVelocity, contactPointLocal0) + rigidBody0.linearVelocity;
		Vec3f totalContactVelocityContact0 = collision.invContactBasis.Transposed() * totalContactVelocityLocal0;

		Vec3f accVelocity0 = rigidBody0.lastAcceleration * stepTime;
		accVelocity0 = collision.invContactBasis.Transposed() * accVelocity0;
		accVelocity0.x = 0;

		totalContactVelocityContact0 += accVelocity0; // NOTE: negated test?

		/* Body 1 */

		Vec3f contactPointLocal1 = collision.contact1[0] - transform1.position;
		Vec3f totalContactVelocityLocal1 = Cross(rigidBody1.angularVelocity, contactPointLocal1) + rigidBody1.linearVelocity;
		Vec3f totalContactVelocityContact1 = collision.invContactBasis.Transposed() * totalContactVelocityLocal1;

		Vec3f accVelocity1 = rigidBody1.lastAcceleration * stepTime;
		accVelocity1 = collision.invContactBasis.Transposed() * accVelocity1;
		accVelocity1.x = 0;

		totalContactVelocityContact1 += accVelocity1;

		return totalContactVelocityContact0 - totalContactVelocityContact1;
	}

	float CalcDesiredDeltaVelocity(const Collision& collision, const RigidBody& rigidBody0, const RigidBody& rigidBody1, 
		const Transform& transform0, const Transform& transform1, double stepTime)
	{
		float velFromAccel = 0;

		Vec3f contactVelocity = CalcContactVelocity(collision, rigidBody0, rigidBody1, transform0, transform1, stepTime); // @TODO: move out of here

		if (!rigidBody0.asleep)
		{
			velFromAccel += Dot(rigidBody0.lastAcceleration * stepTime, collision.normal); // NOTE: Negated test
		}

		if (!rigidBody1.asleep) // @TODO static checks?
		{
			velFromAccel += Dot(rigidBody1.lastAcceleration * stepTime, collision.normal);
		}

		// @TODO properly calculate restition
		float restitution = Min(rigidBody0.restitution, rigidBody1.restitution);

		//return -contactVelocity.x - restitution * (contactVelocity.x - velFromAccel);
		return -contactVelocity.x + (1.0f + restitution);
	}

	Vec3f CalcImpulse(const Collision& collision, const RigidBody& rigidBody0, const RigidBody& rigidBody1, const Vec3f& contactNormal,
		const Transform& transform0, const Transform& transform1, double stepTime)
	{
		float angularComponent = 0;
		float desiredAngularComponent = CalcDesiredDeltaVelocity(collision, rigidBody0, rigidBody1, transform0, transform1, stepTime);

		Vec3f contact0 = collision.contact0[0] - transform0.position;
		Vec3f torqueImpulse0 = Cross(contact0, contactNormal);
		Vec3f angularVelocityImpulse0 = rigidBody0.invInertiaTensor * torqueImpulse0;
		Vec3f velocityImpulse0 = Cross(angularVelocityImpulse0, contact0);
		angularComponent += Dot(velocityImpulse0, contactNormal) + rigidBody0.invMass; // NOTE: negated test?

		Vec3f contact1 = collision.contact1[0] - transform1.position;
		Vec3f torqueImpulse1 = Cross(contact1, contactNormal);
		Vec3f angularVelocityImpulse1 = rigidBody1.invInertiaTensor * torqueImpulse1;
		Vec3f velocityImpulse1 = Cross(angularVelocityImpulse1, contact1);
		angularComponent += Dot(velocityImpulse1, contactNormal) + rigidBody1.invMass;

		Vec3f contactImpulse = Vec3f(desiredAngularComponent / angularComponent, 0.0f, 0.0f);

		Vec3f impulse = collision.invContactBasis * contactImpulse;

		return impulse;
	}

	void Physics::ApplyImpulses(EntityWorld& world, RigidBodyView& rigidBodies, double stepTime)
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

			rigidBody0.invInertiaTensor = Mat3f().SetIdentity();
			rigidBody1.invInertiaTensor = Mat3f().SetIdentity();

			ResolveCollisionDepths(collider0, collider1, normal * depth, transform0, transform1);
			Vec3f impulse = CalcImpulse(collision, rigidBody0, rigidBody1, normal, transform0, transform1, stepTime);

			//float j = CalcResolvedForces(rigidBody0, rigidBody1, scaledNormal);
			//Vec3f linearImpulse = j * scaledNormal;

			if (!collider0.isStatic)
			{
				//rigidBody0.AddLinearVelocity(-linearImpulse * rigidBody0.invMass);

				Vec3f contact0 = collision.contact0[0] - transform0.position;
				Vec3f impulseTorque = Cross(contact0, impulse);
				Vec3f finalAngularVelocity = rigidBody0.invInertiaTensor * impulseTorque;
				Vec3f finalLinearVelocity = impulse * rigidBody0.invMass;

				rigidBody0.AddLinearVelocity(finalLinearVelocity);
				rigidBody0.AddAngularVelocity(finalAngularVelocity);
			}

			if (!collider1.isStatic)
			{
				//rigidBody1.AddLinearVelocity(linearImpulse * rigidBody1.invMass);

				Vec3f contact1 = collision.contact1[0] - transform1.position;
				Vec3f impulseTorque = Cross(contact1, impulse);
				Vec3f finalAngularVelocity = rigidBody1.invInertiaTensor * impulseTorque;
				Vec3f finalLinearVelocity = impulse * -rigidBody1.invMass;

				rigidBody1.AddLinearVelocity(finalLinearVelocity);
				rigidBody1.AddAngularVelocity(finalAngularVelocity);
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

