#include "Physics.h"

namespace Quartz
{
	Physics::Line::Line() :
		points{} {}

	Physics::Line::Line(const Vec3f& a, const Vec3f& b) :
		points{ a, b } {}

	Physics::Triangle::Triangle() :
		points{} {}

	Physics::Triangle::Triangle(const Vec3f& a, const Vec3f& b, const Vec3f& c) :
		points{ a, b, c } {}

	Physics::Tetrahedron::Tetrahedron() :
		points{} {};

	Physics::Tetrahedron::Tetrahedron(const Vec3f& a, const Vec3f& b, const Vec3f& c, const Vec3f& d) :
		points{ a, b, c, d } {};

	Physics::Polytope::Polytope() :
		mTris{}, mSize(0) {}

	bool Physics::Polytope::AddSimplex(const Simplex& simplex)
	{
		if (simplex.Size() != 4)
		{
			return false;
		}

		Vec3f a = simplex[3];
		Vec3f b = simplex[2];
		Vec3f c = simplex[1];
		Vec3f d = simplex[0];

		AddTriangle(Triangle(a, c, b));
		AddTriangle(Triangle(a, d, c));
		AddTriangle(Triangle(a, b, d));
		AddTriangle(Triangle(b, c, d));

		return true;
	}

	void Physics::Polytope::AddTriangle(const Triangle& tri)
	{
		// @TODO assert mSize < PHYSICS_EPA_MAX_TRIANGLES
		mTris[mSize++] = tri;
	}

	void Physics::Polytope::RemoveTriangle(uSize index)
	{
		// @TODO assert index < mSize
		mTris[index] = mTris[mSize - 1];
		mSize--;
	}

	Vec3f CalcTriangleNormal(const Physics::Triangle& tri)
	{
		constexpr Vec3f bias = Vec3f{ 1.0f, 0.0f, 0.0f };

		Vec3f ab = tri.points[1] - tri.points[0];
		Vec3f ac = tri.points[2] - tri.points[0];

		Vec3f normal = Cross(ab, ac);

		if (normal == Vec3f::ZERO) // Not a triangle
		{
			if (tri.points[1] == tri.points[2]) // Line
			{
				if (tri.points[0] == tri.points[1]) // Unison
				{
					return bias;
				}

				normal = Cross(ab, ab + bias);
			}
			else // Line, opposite directions
			{
				Vec3f bc = tri.points[2] - tri.points[1];
				normal = Cross(bc, bc + bias);
			}
		}

		return normal.Normalized();
	}

	float DistanceToPlane(const Vec3f& point, const Vec3f& planePoint, const Vec3f& normal)
	{
		Vec3f diff = point - planePoint;
		return fabsf(Dot(diff, normal));
	}

	void Physics::Polytope::ClosestTriangle(const Vec3f& point, Triangle& outTri, uSize& outIndex, float& outDist, Vec3f& outNormal) const
	{
		Triangle closestTri		= {};
		float closestDist		= FLT_MAX;
		uSize closestIndex		= 0;
		Vec3f closestNormal		= {};

		for (uSize i = 0; i < mSize; i++)
		{
			Vec3f normal = CalcTriangleNormal(mTris[i]);
			float dist = DistanceToPlane(point, mTris[i].points[0], normal);

			if (dist < closestDist)
			{
				closestTri		= mTris[i];
				closestDist		= dist;
				closestIndex	= i;
				closestNormal	= normal;
			}
		}

		outTri		= closestTri;
		outIndex	= closestIndex;
		outDist		= closestDist;
		outNormal	= closestNormal;
	}

	void Physics::Polytope::Extend(const Vec3f& point)
	{
		Line	lines[PHYSICS_EPA_MAX_EDGES];
		uSize	lineCount = 0;
		Simplex	contactSimplex;

		for (uSize i = 0; i < mSize; i++)
		{
			Vec3f a = mTris[i].points[0];
			Vec3f b = mTris[i].points[1];
			Vec3f c = mTris[i].points[2];

			Vec3f normal = CalcTriangleNormal(mTris[i]);

			if (Dot(normal, point - a) > 0.0f)
			{
				RemoveTriangle(i);
				i--; // Because we swap tris when removing;

				Line ab(a, b);
				Line bc(b, c);
				Line ca(c, a);

				bool abDup = false;
				bool bcDup = false;
				bool caDup = false;

				for (uSize j = 0; j < lineCount; j++)
				{
					Line line = lines[j];
					bool removeLine = false;

					if (line.points[0] == ab.points[1] && line.points[1] == ab.points[0])
					{
						abDup = true;
						removeLine = true;
					}

					if (line.points[0] == bc.points[1] && line.points[1] == bc.points[0])
					{
						bcDup = true;
						removeLine = true;
					}

					if (line.points[0] == ca.points[1] && line.points[1] == ca.points[0])
					{
						caDup = true;
						removeLine = true;
					}

					if (removeLine)
					{
						lines[j] = lines[lineCount - 1];
						lineCount--;
						j--;
					}
				}

				if (!abDup)
				{
					lines[lineCount] = ab;
					lineCount++;
				}

				if (!bcDup)
				{
					lines[lineCount] = bc;
					lineCount++;
				}

				if (!caDup)
				{
					lines[lineCount] = ca;
					lineCount++;
				}
			}
		}
		for (uSize i = 0; i < lineCount; i++)
		{
			Triangle newTriangle(lines[i].points[0], lines[i].points[1], point);
			AddTriangle(newTriangle);
		}
	}

	Vec3f Physics::MinkowskiFurthestPoint(const Collider& collider0, const Collider& collider1, const Vec3f direction)
	{
		return FurthestPoint(collider0, direction) - FurthestPoint(collider1, -direction);
	}

	Vec3f Physics::MinkowskiFurthestPointRect(const Collider& collider0, const Collider& rect1, Vec3f(&points)[8], const Vec3f direction)
	{
		return FurthestPointSphere(collider0, direction) - FurthestPointRect(rect1, -direction, points);
	}

	Vec3f Physics::MinkowskiFurthestPointRectRect(const Collider& rect0, Vec3f(&points0)[8], const Collider& rect1, Vec3f(&points1)[8], const Vec3f direction)
	{
		return FurthestPointRect(rect0, direction, points0) - FurthestPointRect(rect1, -direction, points1);
	}

	bool Physics::GJK(const Collider& collider0, const Collider& collider1, Simplex& outSimplex)
	{
		Simplex simplex;

		// Check any inital direction (x-axis) for the furthest point in minkowski difference.
		Vec3f direction = Vec3f::X_AXIS;
		Vec3f furthestPoint = MinkowskiFurthestPoint(collider0, collider1, direction);

		// If the initial minkowski point is zero, 
		// the colliders are touching but not overlapping. Do not collide.
		if (furthestPoint.IsZero())
		{
			// @TODO
			return false;
		}

		// Add the inital point to the simplex.
		simplex.Push(furthestPoint);

		// Search in the opposite direction of the inital point.
		direction = -furthestPoint;

		uSize iter = 0;
		while (iter++ < PHYSICS_GJK_MAX_ITERATIONS)
		{
			furthestPoint = MinkowskiFurthestPoint(collider0, collider1, direction);

			// Is the origin outside the search region
			if (Dot(furthestPoint, direction) <= 0)
			{
				return false; // No Collision
			}

			// Add the new point to the simplex.
			simplex.Push(furthestPoint);

			// Get the next direction from the simplex
			if (simplex.Next(direction))
			{
				outSimplex = simplex;
				return true;
			}
		}

		return false; // Exceeded max iterations
	}

	bool Physics::GJKRect(const Collider& collider0, const Collider& rect1, Vec3f(&points)[8], Simplex& outSimplex)
	{
		Simplex simplex;

		// Check any inital direction (x-axis) for the furthest point in minkowski difference.
		Vec3f direction = Vec3f::X_AXIS;
		Vec3f furthestPoint = MinkowskiFurthestPointRect(collider0, rect1, points, direction);

		// If the initial minkowski point is zero, 
		// the colliders are touching but not overlapping. Do not collide.
		if (furthestPoint.IsZero())
		{
			// @TODO
			return false;
		}

		// Add the inital point to the simplex.
		simplex.Push(furthestPoint);

		// Search in the opposite direction of the inital point.
		direction = -furthestPoint;

		uSize iter = 0;
		while (iter++ < PHYSICS_GJK_MAX_ITERATIONS)
		{
			furthestPoint = MinkowskiFurthestPointRect(collider0, rect1, points, direction);

			// Is the origin outside the search region
			if (Dot(furthestPoint, direction) < 0.0f)
			{
				return false; // No Collision
			}

			// Add the new point to the simplex.
			simplex.Push(furthestPoint);

			// Get the next direction from the simplex
			if (simplex.Next(direction))
			{
				outSimplex = simplex;
				return true;
			}
		}

		return false; // Exceeded max iterations
	}

	bool Physics::GJKRectRect(const Collider& rect0, Vec3f(&points0)[8], const Collider& rect1, Vec3f(&points1)[8], Simplex& outSimplex)
	{
		Simplex simplex;

		// Check any inital direction (x-axis) for the furthest point in minkowski difference.
		Vec3f direction = Vec3f::X_AXIS;
		Vec3f furthestPoint = MinkowskiFurthestPointRectRect(rect0, points0, rect1, points1, direction);

		// If the initial minkowski point is zero, 
		// the colliders are touching but not overlapping. Do not collide.
		if (furthestPoint.IsZero())
		{
			// @TODO
			return false;
		}

		// Add the inital point to the simplex.
		simplex.Push(furthestPoint);

		// Search in the opposite direction of the inital point.
		direction = -furthestPoint;

		uSize iter = 0;
		while (iter++ < PHYSICS_GJK_MAX_ITERATIONS)
		{
			furthestPoint = MinkowskiFurthestPointRectRect(rect0, points0, rect1, points1, direction);

			// Is the origin outside the search region
			if (Dot(furthestPoint, direction) < 0.0f)
			{
				return false; // No Collision
			}

			// Add the new point to the simplex.
			simplex.Push(furthestPoint);

			// Get the next direction from the simplex
			if (simplex.Next(direction))
			{
				outSimplex = simplex;
				return true;
			}
		}

		return false; // Exceeded max iterations
	}

	Collision Physics::EPA(const Collider& collider0, const Collider& collider1, const Simplex& simplex)
	{
		constexpr float tolerance = 0.01f;

		Polytope polytope;
		polytope.AddSimplex(simplex);

		uSize iteration = 0;
		while (iteration++ < PHYSICS_EPA_MAX_ITERATIONS)
		{
			Triangle	tri;
			uSize		index;
			float		dist;
			Vec3f		normal;

			// Find the triangle closest to the origin
			polytope.ClosestTriangle(Vec3f::ZERO, tri, index, dist, normal);

			//normal.Normalize();

			// Get the furthest point in the normal direction
			Vec3f furthestPoint = MinkowskiFurthestPoint(collider0, collider1, normal);

			if (Dot(furthestPoint, normal) > dist + tolerance)
			{
				polytope.Extend(furthestPoint);
			}
			else
			{
				//Vec3f extent0 = FurthestPoint(collider0, normal);
				//Vec3f extent1 = extent0 - normal.Normalized() * dist;

				Simplex contact0 = FurthestSimplex(collider0, -normal);
				Simplex contact1 = FurthestSimplex(collider1, normal);

				//return Collision(normal.Normalized(), dist, contact0, contact1);
				return Collision(); // No Collision
			}
		}

		return Collision();
	}

	Collision Physics::EPARect(const Collider& collider0, const Collider& rect1, Vec3f(&points)[8], const Simplex& simplex)
	{
		constexpr float tolerance = 0.01f;

		Polytope polytope;
		polytope.AddSimplex(simplex);

		uSize iteration = 0;
		while (iteration++ < PHYSICS_EPA_MAX_ITERATIONS)
		{
			Triangle	tri;
			uSize		index;
			float		dist;
			Vec3f		normal;

			// Find the triangle closest to the origin
			polytope.ClosestTriangle(Vec3f::ZERO, tri, index, dist, normal);

			// Get the furthest point in the normal direction
			Vec3f furthestPoint = MinkowskiFurthestPointRect(collider0, rect1, points, normal);

			if (Dot(furthestPoint, normal) > dist + tolerance)
			{
				polytope.Extend(furthestPoint);
			}
			else
			{
				Simplex contact0 = FurthestSimplex(collider0, -normal);
				Simplex contact1 = FurthestSimplexRect(rect1, normal, points);

				Collision collision(normal.Normalized(), dist);

				if (contact1.Size() > contact0.Size())
				{
					contact0 = contact1;
				}

				for (uSize i = 0; i < contact0.Size(); i++)
				{
					collision.AddContact(contact0[i]);
				}

				return collision;
			}
		}

		return Collision();
	}

	Collision Physics::EPARectRect(const Collider& rect0, Vec3f(&points0)[8], const Collider& rect1, Vec3f(&points1)[8], const Simplex& simplex)
	{
		constexpr float tolerance = 0.01f;

		Polytope polytope;
		polytope.AddSimplex(simplex);

		uSize iteration = 0;
		while (iteration++ < PHYSICS_EPA_MAX_ITERATIONS)
		{
			Triangle	tri;
			uSize		index;
			float		dist;
			Vec3f		normal;

			// Find the triangle closest to the origin
			polytope.ClosestTriangle(Vec3f::ZERO, tri, index, dist, normal);

			// Get the furthest point in the normal direction
			Vec3f furthestPoint = MinkowskiFurthestPointRectRect(rect0, points0, rect1, points1, normal);

			if (Dot(furthestPoint, normal) > dist + tolerance)
			{
				polytope.Extend(furthestPoint);
			}
			else
			{
				//Vec3f extent0 = FurthestPoint(rect0, normal);
				//Vec3f extent1 = extent0 - normal.Normalized() * dist;

				Simplex contact0 = FurthestSimplexRect(rect0, -normal, points0);
				Simplex contact1 = FurthestSimplexRect(rect1, normal, points1);

				//return Collision(normal.Normalized(), dist, contact0, contact1);
				return Collision(); // No Collision
			}
		}

		return Collision();
	}
}