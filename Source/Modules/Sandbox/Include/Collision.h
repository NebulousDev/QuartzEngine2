#pragma once

#include "Simplex.h"
#include "PhysicsTypes.h"
#include "Math/Matrix.h"

#define PHYSICS_MAX_CONTACT_POINTS 6

namespace Quartz
{
	struct Contact
	{
		Vec3p	point;
		floatp	depth;
		Vec3p	normal;
		Vec3p	localPoint0;
		Vec3p	localPoint1;
		Vec3p	contactVelocity;
		floatp	targetVelocity;
		Mat3p	invContactBasis;

		Contact();
		Contact(const Vec3p& point, floatp depth, const Vec3p& normal);

		Contact& Flip();
		void CalcLocalPoints(const Vec3p& position0, const Vec3p& position1);
		void CalcContactVelocity(const Vec3p& lastAccel0, const Vec3p& lastAccel1, 
			const Vec3p& angularVel0, const Vec3p& angularVel1, 
			const Vec3p& linearVel0, const Vec3p& linearVel1, double stepTime);
		void CalcTargetVelocity(const Vec3p& lastAccel0, const Vec3p& lastAccel1, 
			floatp restitution0, floatp restitution1, double timeStep);
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