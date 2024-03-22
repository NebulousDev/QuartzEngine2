#include "Collision.h"
#include "Utility/Swap.h"

namespace Quartz
{
	Collision::Collision(const Vec3f& normal, float depth, bool colliding) :
		normal(normal), depth(depth), count(0), isColliding(colliding)
	{
		RecalcContactBasis();
	};

	void Collision::AddContact(const Vec3f& point)
	{
		//@TODO assert point count < PHYSICS_MAX_CONTACT_POINTS
		points[count++] = point;
	}

	// @NOTE: Remember to call RecalcContactBasis() after flipping
	Collision& Collision::Flip()
	{
		normal = -normal;
		return *this;
	}

	void Collision::RecalcContactBasis()
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
}