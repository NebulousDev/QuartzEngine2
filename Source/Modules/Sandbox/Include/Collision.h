#pragma once

#include "Simplex.h"
#include "Math/Matrix.h"

namespace Quartz
{
	struct Collision
	{
		Vec3f	normal;
		float	depth;
		Simplex contact0;
		Simplex contact1;

		Mat3f	invContactBasis;
		Vec3f	contactVelocity;
		float	desiredDeltaVelocity;
		bool	isColliding;

		Collision() = default;
		Collision(const Vec3f& normal, float depth, const Simplex& contact0, const Simplex& contact1, bool colliding = true);

		Collision& Flip();

		void RecalcContactBasis();
	};
}