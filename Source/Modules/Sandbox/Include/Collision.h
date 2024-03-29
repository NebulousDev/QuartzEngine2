#pragma once

#include "Simplex.h"
#include "Math/Matrix.h"

#define PHYSICS_MAX_CONTACT_POINTS 6

namespace Quartz
{
	struct Contact
	{
		Vec3f point;
		float depth;
		Vec3f normal;
		Vec3f localPoint0;
		Vec3f localPoint1;
		Vec3f contactVelocity;
		float targetVelocity;
		Mat3f invContactBasis;

		Contact();
		Contact(const Vec3f& point, float depth, const Vec3f& normal);

		Contact& Flip();
		void CalcLocalPoints(const Vec3f& position0, const Vec3f& position1);
		void CalcContactVelocity(const Vec3f& lastAccel0, const Vec3f& lastAccel1, 
			const Vec3f& angularVel0, const Vec3f& angularVel1, 
			const Vec3f& linearVel0, const Vec3f& linearVel1, double stepTime);
		void CalcTargetVelocity(const Vec3f& lastAccel0, const Vec3f& lastAccel1, 
			float restitution0, float restitution1, double timeStep);
		void CalcContactBasis();
	};

	struct Collision
	{
		Contact	contacts[PHYSICS_MAX_CONTACT_POINTS];
		uSize	count;

		Collision();

		void AddContact(const Contact& contact);
		Collision& Flip();
	};
}