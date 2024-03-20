#include "Collision.h"
#include "Utility/Swap.h"

namespace Quartz
{
	Collision::Collision(const Vec3f& normal, float depth, const Simplex& contact0, const Simplex& contact1, bool colliding) :
		normal(normal), depth(depth), contact0(contact0), contact1(contact1), isColliding(colliding)
	{
		RecalcContactBasis();
	};

	Collision& Collision::Flip()
	{
		normal = -normal;
		Swap(contact0, contact1);

		RecalcContactBasis();

		return *this;
	}

	void Collision::RecalcContactBasis()
	{
		Vec3f tangentY;
		Vec3f tangentZ;

		if (Abs(normal.x) > Abs(normal.y))
		{
			float s = FastInvsereSquare(normal.z * normal.z + normal.x * normal.x);

			tangentZ.x = normal.z * s;
			tangentZ.y = 0;
			tangentZ.z = -normal.x * s;

			tangentY.x = normal.y * tangentZ.x;
			tangentY.y = normal.z * tangentZ.x - normal.x * tangentZ.z;
			tangentY.z = -normal.y * tangentZ.x;
		}
		else
		{
			float s = FastInvsereSquare(normal.z * normal.z + normal.y * normal.y);

			tangentZ.x = 0;
			tangentZ.y = -normal.z * s;
			tangentZ.z = normal.y * s;

			tangentY.x = normal.y * tangentZ.z - normal.z * tangentZ.y;
			tangentY.y = -normal.x * tangentZ.z;
			tangentY.z = normal.x * tangentZ.y;
		}

		invContactBasis = Mat3f(normal, tangentZ, tangentY);
	}
}