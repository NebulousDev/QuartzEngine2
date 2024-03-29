#include "GJK.h"
#include <float.h>

namespace Quartz
{
	using namespace ShapeUtils;

	GJK::Line::Line() :
		points{} {}

	GJK::Line::Line(const Vec3p& a, const Vec3p& b) :
		points{ a, b } {}

	GJK::Triangle::Triangle() :
		points{} {}

	GJK::Triangle::Triangle(const Vec3p& a, const Vec3p& b, const Vec3p& c) :
		points{ a, b, c } {}

	GJK::Tetrahedron::Tetrahedron() :
		points{} {};

	GJK::Tetrahedron::Tetrahedron(const Vec3p& a, const Vec3p& b, const Vec3p& c, const Vec3p& d) :
		points{ a, b, c, d } {};

	GJK::Polytope::Polytope() :
		mTris{}, mSize(0) {}

	bool GJK::Polytope::AddSimplex(const Simplex& simplex)
	{
		if (simplex.Size() != 4)
		{
			return false;
		}

		const Vec3p a = simplex[3];
		const Vec3p b = simplex[2];
		const Vec3p c = simplex[1];
		const Vec3p d = simplex[0];

		AddTriangle(Triangle(a, c, b));
		AddTriangle(Triangle(a, d, c));
		AddTriangle(Triangle(a, b, d));
		AddTriangle(Triangle(b, c, d));

		return true;
	}

	void GJK::Polytope::AddTriangle(const Triangle& tri)
	{
		// @TODO assert mSize < PHYSICS_EPA_MAX_TRIANGLES
		mTris[mSize++] = tri;
	}

	void GJK::Polytope::RemoveTriangle(uSize index)
	{
		// @TODO assert index < mSize
		mTris[index] = mTris[mSize - 1];
		mSize--;
	}

	Vec3p CalcTriangleNormal(const GJK::Triangle& tri)
	{
		constexpr Vec3p bias = Vec3p{ 1.0f, 0.0f, 0.0f };

		const Vec3p ab = tri.points[1] - tri.points[0];
		const Vec3p ac = tri.points[2] - tri.points[0];

		Vec3p normal = Cross(ab, ac);

		if (normal == Vec3p::ZERO) // Not a triangle
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
				Vec3p bc = tri.points[2] - tri.points[1];
				normal = Cross(bc, bc + bias);
			}
		}

		return normal.Normalized();
	}

	floatp DistanceToPlane(const Vec3p& point, const Vec3p& planePoint, const Vec3p& normal)
	{
		const Vec3p diff = point - planePoint;
		return fabsf(Dot(diff, normal));
	}

	void GJK::Polytope::ClosestTriangle(const Vec3p& point, Triangle& outTri, uSize& outIndex, floatp& outDist, Vec3p& outNormal) const
	{
		Triangle closestTri		= {};
		floatp closestDist		= FLT_MAX;
		uSize closestIndex		= 0;
		Vec3p closestNormal		= {};

		for (uSize i = 0; i < mSize; i++)
		{
			Vec3p normal = CalcTriangleNormal(mTris[i]);
			floatp dist = DistanceToPlane(point, mTris[i].points[0], normal);

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

	void GJK::Polytope::Extend(const Vec3p& point)
	{
		Line	lines[PHYSICS_EPA_MAX_EDGES];
		uSize	lineCount = 0;
		Simplex	contactSimplex;

		for (uSize i = 0; i < mSize; i++)
		{
			Vec3p a = mTris[i].points[0];
			Vec3p b = mTris[i].points[1];
			Vec3p c = mTris[i].points[2];

			Vec3p normal = CalcTriangleNormal(mTris[i]);

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
}