#include "Collision.h"
#include "Utility/Swap.h"

namespace Quartz
{
	Contact::Contact() :
		point(), depth(), normal() {}

	Contact::Contact(const Vec3p& point, floatp depth, const Vec3p& normal) :
		point(point), depth(depth), normal(normal) { }

	Contact& Contact::Flip()
	{
		normal = -normal;
		return *this;
	}

	void Contact::CalcLocalPoints(const Vec3p& position0, const Vec3p& position1)
	{
		localPoint0 = point - position0;
		localPoint1 = point - position1;
	}

	void Contact::CalcContactVelocity(const Vec3p& lastAccel0, const Vec3p& lastAccel1, 
		const Vec3p& angularVel0, const Vec3p& angularVel1, const Vec3p& linearVel0, const Vec3p& linearVel1, double stepTime)
	{
		const Vec3p velocity0 = Cross(angularVel0, localPoint0) + linearVel0;
		const Vec3p velocity1 = Cross(angularVel1, localPoint1) + linearVel1;

		Vec3p contactVelocity0 = invContactBasis.Transposed() * velocity0; // Move basis down
		Vec3p contactVelocity1 = invContactBasis.Transposed() * velocity1;

		Vec3p accelVelocity0 = invContactBasis.Transposed() * (lastAccel0 * stepTime);
		Vec3p accelVelocity1 = invContactBasis.Transposed() * (lastAccel1 * stepTime);

		accelVelocity0.x = 0.0f;
		accelVelocity1.x = 0.0f;

		contactVelocity0 += accelVelocity0;
		contactVelocity1 += accelVelocity1;

		contactVelocity = contactVelocity0 - contactVelocity1;
	}

	void Contact::CalcTargetVelocity(const Vec3p& lastAccel0, const Vec3p& lastAccel1, floatp restitution0, floatp restitution1, double stepTime)
	{
		const floatp accelVelocity0 = Dot(lastAccel0 * stepTime, normal);
		const floatp accelVelocity1 = Dot(lastAccel1 * stepTime, normal);
		const floatp accelVelocity = accelVelocity0 - accelVelocity1;

		floatp restitution = Min(restitution0, restitution1);

		if (Abs(contactVelocity.x) < 0.25f) // Velocity limit
		{
			restitution = 0.0f;
		}

		targetVelocity = -contactVelocity.x - restitution * (contactVelocity.x - accelVelocity);
	}

	void Contact::CalcContactBasis()
	{
		Vec3p tangentZ;
		Vec3p tangentY;

		if (Abs(normal.x) > Abs(normal.y))
		{
			const floatp s = FastInvsereSquare(normal.z * normal.z + normal.x * normal.x);

			tangentZ.x = normal.z * s;
			tangentZ.y = 0;
			tangentZ.z = -normal.x * s;

			tangentY.x = normal.y * tangentZ.x;
			tangentY.y = normal.z * tangentZ.x - normal.x * tangentZ.z;
			tangentY.z = -normal.y * tangentZ.x;
		}
		else
		{
			const floatp s = FastInvsereSquare(normal.z * normal.z + normal.y * normal.y);

			tangentZ.x = 0;
			tangentZ.y = -normal.z * s;
			tangentZ.z = normal.y * s;

			tangentY.x = normal.y * tangentZ.z - normal.z * tangentZ.y;
			tangentY.y = -normal.x * tangentZ.z;
			tangentY.z = normal.x * tangentZ.y;
		}

		invContactBasis = Mat3p().SetRows(normal, tangentZ, tangentY);
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