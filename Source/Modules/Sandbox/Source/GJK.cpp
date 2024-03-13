#include "Physics.h"

namespace Quartz
{
	Physics::Simplex::Simplex() : mSize(0), mPoints{} {}

	void Physics::Simplex::Push(const Vec3f& point)
	{
		//mPoints[3] = mPoints[2];
		//mPoints[2] = mPoints[1];
		//mPoints[1] = mPoints[0];
		//mPoints[0] = point;

		//mSize = mSize == 4 ? 4 : mSize + 1;

		mPoints[mSize++] = point;
	}

	uSize Physics::Simplex::Size() const
	{
		return mSize;
	}

	bool Physics::Simplex::Line(Vec3f& inOutDir)
	{
		Vec3f a = mPoints[1];
		Vec3f b = mPoints[0];

		Vec3f ab = b - a;
		Vec3f ao = -a;

		if (Dot(ab, ao) > 0.0f)
		{
			inOutDir = Cross(Cross(ab, ao), ab);
		}

		/*
		else
		{
			mPoints[0]	= a;
			mSize		= 1;
			inOutDir	= ao;
		}
		*/

		return false;
	}

	bool Physics::Simplex::Triangle(Vec3f& inOutDir)
	{
		Vec3f a = mPoints[2];
		Vec3f b = mPoints[1];
		Vec3f c = mPoints[0];

		Vec3f ab = b - a;
		Vec3f ac = c - a;
		Vec3f ao = -a;
 
		Vec3f abc = Cross(ab, ac);

		if (Dot(Cross(abc, ac), ao) > 0.0f)
		{
			if (Dot(ac, ao) > 0.0f)
			{
				mPoints[0] = a;
				mPoints[1] = c;
				mSize = 2;

				inOutDir = Cross(Cross(ac, ao), ac);
			}

			else
			{
				if (Dot(ab, ao) > 0.0f)
				{
					mPoints[0] = a;
					mPoints[1] = b;
					mSize = 2;

					inOutDir = Cross(Cross(ab, ao), ab);
				}
				else
				{
					mPoints[0] = a;
					mSize = 1;

					inOutDir = ao;
				}
			}
		}
 
		else if (Dot(Cross(ab, abc), ao) > 0.0f)
		{
			if (Dot(ab, ao) > 0.0f)
			{
				mPoints[0] = a;
				mPoints[1] = b;
				mSize = 2;

				inOutDir = Cross(Cross(ab, ao), ab);
			}
			else
			{
				mPoints[0] = a;
				mSize = 1;

				inOutDir = ao;
			}
		}

		else // Inside the triangle
		{
			if (Dot(abc, ao) > 0.0f)
			{
				mPoints[0] = b;
				mPoints[1] = c;
				mPoints[2] = a;
				mSize = 3;

				inOutDir = abc;
			}

			else // Opposite winding
			{
				inOutDir = -abc;
			}
		}

		return false;
	}

	bool Physics::Simplex::Tetrahedron(Vec3f& inOutDir)
	{
		Vec3f a = mPoints[3];
		Vec3f b = mPoints[2];
		Vec3f c = mPoints[1];
		Vec3f d = mPoints[0];

		Vec3f ab = b - a;
		Vec3f ac = c - a;
		Vec3f ad = d - a;
		Vec3f ao = -a;
 
		Vec3f acb = Cross(ac, ab);
		Vec3f adc = Cross(ad, ac);
		Vec3f abd = Cross(ab, ad);
 
		if (Dot(acb, ao) > 0.0f)
		{
			mPoints[0] = a;
			mPoints[1] = c;
			mPoints[2] = b;
			mSize = 3;

			return Triangle(inOutDir);
		}
		
		else if (Dot(adc, ao) > 0.0f)
		{
			mPoints[0] = a;
			mPoints[1] = d;
			mPoints[2] = c;
			mSize = 3;

			return Triangle(inOutDir);
		}
 
		else if (Dot(abd, ao) > 0.0f)
		{
			mPoints[0] = a;
			mPoints[1] = b;
			mPoints[2] = d;
			mSize = 3;

			return Triangle(inOutDir);
		}
 
		return true;
	}

	bool Physics::Simplex::Next(Vec3f& inOutDir)
	{
		switch (mSize)
		{
			case 2: return Line(inOutDir);
			case 3: return Triangle(inOutDir);
			case 4: return Tetrahedron(inOutDir);
			default: return false;
		}
	}

	const Vec3f& Physics::Simplex::operator[](uSize index) const
	{
		return mPoints[index];
	}

	Physics::Line::Line() :
		points{} {}

	Physics::Line::Line(const Vec3f& a, const Vec3f& b) :
		points{ a, b } {}

	Physics::Triangle::Triangle() :
		points{} {}

	Physics::Triangle::Triangle(const Vec3f& a, const Vec3f& b, const Vec3f& c) :
		points{a, b, c} {}

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

	void Physics::Polytope::ClosestTriangle(const Vec3f& point, Triangle& outTri, uSize& outIndex, float& outDist, Vec3f& outNormal)
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
		Line lines[PHYSICS_EPA_MAX_TRIANGLES * 3];
		uSize lineCount = 0;

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
			AddTriangle(Triangle(lines[i].points[0], lines[i].points[1], point));
		}
	}

	Vec3f Physics::MinkowskiFurthestPoint(const Collider& collider0, const Collider& collider1, const Vec3f direction)
	{
		return FurthestPoint(collider0, direction) - FurthestPoint(collider1, -direction);
	}

	Vec3f Physics::MinkowskiFurthestPointRect(const Collider& collider0, const Collider& rect1, const Vec3f direction, Vec3f(&mPoints)[8])
	{
		return FurthestPointSphere(collider0, direction) - FurthestPointRect(rect1, -direction, mPoints);
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
		Vec3f furthestPoint = MinkowskiFurthestPointRect(collider0, rect1, direction, points);

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
			furthestPoint = MinkowskiFurthestPointRect(collider0, rect1, direction, points);

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

			// Get the furthest point in the normal direction
			Vec3f furthestPoint = MinkowskiFurthestPoint(collider0, collider1, normal);

			if (Dot(furthestPoint, normal) > dist + tolerance)
			{
				polytope.Extend(furthestPoint);
			}
			else
			{
				float dist2 = DistanceToPlane(Vec3f::ZERO, tri.points[0], normal);

				// @TODO: double check
				return Collision(normal.Normalized() * dist2, {0.0f, 0.0f, 0.0f});
			}
		}

		return Collision();
	}
}