#pragma once

#include "Simplex.h"
#include "Math/Matrix.h"

#define PHYSICS_MAX_CONTACT_POINTS 4

namespace Quartz
{
	struct Collision
	{
		Vec3f	normal;
		float	depth;
		Vec3f	points[PHYSICS_MAX_CONTACT_POINTS];
		uSize	count;

		Mat3f	invContactBasis;
		Vec3f	contactVelocity;
		float	desiredDeltaVelocity;
		bool	isColliding;

		Collision() = default;
		Collision(const Vec3f& normal, float depth, bool colliding = true);

		void AddContact(const Vec3f& point);

		Collision& Flip();

		void RecalcContactBasis();
	};
}