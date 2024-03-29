#include "Collision.h"
#include "Utility/Swap.h"

namespace Quartz
{
	Contact::Contact() :
		point(), depth(), normal() {}

	Contact::Contact(const Vec3f& point, float depth, const Vec3f& normal) :
		point(point), depth(depth), normal(normal) { }

	Contact& Contact::Flip()
	{
		normal = -normal;
		return *this;
	}

	void Contact::CalcLocalPoints(const Vec3f& position0, const Vec3f& position1)
	{
		localPoint0 = point - position0;
		localPoint1 = point - position1;
	}

	void Contact::CalcContactVelocity(const Vec3f& lastAccel0, const Vec3f& lastAccel1, 
		const Vec3f& angularVel0, const Vec3f& angularVel1, const Vec3f& linearVel0, const Vec3f& linearVel1, double stepTime)
	{
		const Vec3f velocity0 = Cross(angularVel0, localPoint0) + linearVel0;
		const Vec3f velocity1 = Cross(angularVel1, localPoint1) + linearVel1;

		Vec3f contactVelocity0 = invContactBasis.Transposed() * velocity0; // Move basis down
		Vec3f contactVelocity1 = invContactBasis.Transposed() * velocity1;

		Vec3f accelVelocity0 = invContactBasis.Transposed() * (lastAccel0 * stepTime);
		Vec3f accelVelocity1 = invContactBasis.Transposed() * (lastAccel1 * stepTime);

		accelVelocity0.x = 0.0f;
		accelVelocity1.x = 0.0f;

		contactVelocity0 += accelVelocity0;
		contactVelocity1 += accelVelocity1;

		contactVelocity = contactVelocity0 - contactVelocity1;
	}

	void Contact::CalcTargetVelocity(const Vec3f& lastAccel0, const Vec3f& lastAccel1, float restitution0, float restitution1, double stepTime)
	{
		float accelVelocity0 = Dot(lastAccel0 * stepTime, normal);
		float accelVelocity1 = Dot(lastAccel1 * stepTime, normal);
		float accelVelocity = accelVelocity0 - accelVelocity1;
		float restitution = Min(restitution0, restitution1);

		if (Abs(contactVelocity.x) < 0.25f) // Velocity limit
		{
			restitution = 0.0f;
		}

		targetVelocity = -contactVelocity.x - restitution * (contactVelocity.x - accelVelocity);
	}

	void Contact::CalcContactBasis()
	{
		Vec3f tangentZ;
		Vec3f tangentY;

		if (Abs(normal.x) > Abs(normal.y))
		{
			const float s = FastInvsereSquare(normal.z * normal.z + normal.x * normal.x);

			tangentZ.x = normal.z * s;
			tangentZ.y = 0;
			tangentZ.z = -normal.x * s;

			tangentY.x = normal.y * tangentZ.x;
			tangentY.y = normal.z * tangentZ.x - normal.x * tangentZ.z;
			tangentY.z = -normal.y * tangentZ.x;
		}
		else
		{
			const float s = FastInvsereSquare(normal.z * normal.z + normal.y * normal.y);

			tangentZ.x = 0;
			tangentZ.y = -normal.z * s;
			tangentZ.z = normal.y * s;

			tangentY.x = normal.y * tangentZ.z - normal.z * tangentZ.y;
			tangentY.y = -normal.x * tangentZ.z;
			tangentY.z = normal.x * tangentZ.y;
		}

		invContactBasis = Mat3f().SetRows(normal, tangentZ, tangentY);
	}

	Collision::Collision() : count(0) {}

	void Collision::AddContact(const Contact& contact)
	{
		if (count < PHYSICS_MAX_CONTACT_POINTS)
		{
			contacts[count++] = contact;
		}
	}

	Collision& Collision::Flip()
	{
		for (uSize i = 0; i < count; i++)
		{
			contacts[i].Flip();
		}

		return *this;
	}
}