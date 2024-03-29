#pragma once

#include "Simplex.h"
#include "PhysicsTypes.h"
#include "Math/Math.h"
#include <float.h>

namespace Quartz
{
	enum ShapeType
	{
		SHAPE_NONE		= -1,
		SHAPE_SPHERE	= 0,
		SHAPE_PLANE		= 1,
		SHAPE_RECT		= 2,
		SHAPE_CAPSULE	= 3,
		SHAPE_HULL		= 4,
		SHAPE_MESH		= 5
	};

	struct ShapeSphere
	{
		floatp radius;
	};

	struct ShapePlane
	{
		Vec3p normal;
		floatp width;
		floatp height;
		floatp length;
	};

	struct ShapeRect
	{
		Bounds3f bounds;
	};

	struct ShapeCapsule
	{

	};

	struct ShapeHull
	{

	};

	struct ShapeMesh
	{

	};

	namespace ShapeUtils
	{
		// @NOTE: In all functions, direction is assumed to be normalized

		template<typename... Transforms>
		Vec3p FurthestPoint(const ShapeSphere& sphere, const Vec3p& direction, const Transforms&... transforms)
		{
			return (transforms * ... * direction) * sphere.radius;
		}

		template<typename... Transforms>
		Vec3p FurthestPoint(const ShapePlane& plane, const Vec3p& direction, const Transforms&... transforms)
		{
			return Vec3p::ZERO;
		}

		template<typename... Transforms>
		Vec3p FurthestPoint(const ShapeRect& rect, const Vec3p& direction, Vec3p(&points)[8], const Transforms&... transforms)
		{
			floatp maxDist = -FLT_MAX;
			uSize maxPointIndex = 0;

			for (uSize i = 0; i < 8; i++)
			{
				const Vec3p point = (transforms * ... * points[i]);
				floatp dist = Dot(point, direction);

				if (dist > maxDist)
				{
					maxDist = dist;
					maxPointIndex = i;
				}
			}

			return points[maxPointIndex];
		}

		template<typename... Transforms>
		Vec3p FurthestPoint(const ShapeRect& rect, const Vec3p& direction, const Transforms&... transforms)
		{
			Vec3p points[8]
			{
				rect.bounds.BottomRightFront(),
				rect.bounds.BottomLeftFront(),
				rect.bounds.BottomRightBack(),
				rect.bounds.BottomLeftBack(),
				rect.bounds.TopRightFront(),
				rect.bounds.TopLeftFront(),
				rect.bounds.TopRightBack(),
				rect.bounds.TopLeftBack()
			};

			return FurthestPointRect(rect, direction, points, transforms...);
		}

		template<typename... Transforms>
		Vec3p FurthestPoint(const ShapeCapsule& capsule, const Vec3p& direction, const Transforms&... transforms)
		{
			return Vec3p::ZERO;
		}

		template<typename... Transforms>
		Vec3p FurthestPoint(const ShapeHull& hull, const Vec3p& direction, const Transforms&... transforms)
		{
			return Vec3p::ZERO;
		}

		template<typename... Transforms>
		Vec3p FurthestPoint(const ShapeMesh& mesh, const Vec3p& direction, const Transforms&... transforms)
		{
			return Vec3p::ZERO;
		}

		template<typename... Transforms>
		Simplex FurthestSimplex(const ShapeSphere& sphere, const Vec3p& direction, const Transforms&... transforms)
		{
			Simplex simplex;
			simplex.Push((transforms * ... * direction) * sphere.radius);
			return simplex;
		}

		template<typename... Transforms>
		Simplex FurthestSimplex(const ShapePlane& plane, const Vec3p& direction, const Transforms&... transforms)
		{
			return Simplex();
		}

		template<typename... Transforms>
		Simplex FurthestSimplex(const ShapeRect& rect, const Vec3p& direction, Vec3p(&points)[8], const Transforms&... transforms)
		{
			Simplex simplex;

			floatp maxDist0 = -FLT_MAX;
			floatp maxDist1 = -FLT_MAX;
			floatp maxDist2 = -FLT_MAX;

			uSize maxPointIndex0 = {};
			uSize maxPointIndex1 = {};
			uSize maxPointIndex2 = {};

			Vec3p normDir = direction.Normalized();

			for (uSize i = 0; i < 8; i++)
			{
				const Vec3p point = (transforms * ... * points[i]);
				floatp dist = Dot(point, normDir);

				if (dist > maxDist0)
				{
					maxDist2 = maxDist1;
					maxDist1 = maxDist0;
					maxDist0 = dist;

					maxPointIndex2 = maxPointIndex1;
					maxPointIndex1 = maxPointIndex0;
					maxPointIndex0 = i;
				}
				else if (dist > maxDist1)
				{
					maxDist2 = maxDist1;
					maxDist1 = dist;

					maxPointIndex2 = maxPointIndex1;
					maxPointIndex1 = i;
				}
				else if (dist > maxDist2)
				{
					maxDist2 = dist;
					maxPointIndex2 = i;
				}
			}

			simplex.Push(points[maxPointIndex0]);

			if (Abs(maxDist0 - maxDist1) < 0.001f)
			{
				if (Abs(maxDist0 - maxDist2) < 0.001f)
				{
					simplex.Push(points[maxPointIndex1]);
					simplex.Push(points[maxPointIndex2]);
				}
				else
				{
					simplex.Push(points[maxPointIndex1]);
				}
			}
			else
			{
				if (Abs(maxDist0 - maxDist2) < 0.001f)
				{
					simplex.Push(points[maxPointIndex2]);
				}
			}

			return simplex;
		}

		template<typename... Transforms>
		Simplex FurthestSimplex(const ShapeRect& rect, const Vec3p& direction, const Transforms&... transforms)
		{
			Vec3p points[8]
			{
				rect.bounds.BottomRightFront(),
				rect.bounds.BottomLeftFront(),
				rect.bounds.BottomRightBack(),
				rect.bounds.BottomLeftBack(),
				rect.bounds.TopRightFront(),
				rect.bounds.TopLeftFront(),
				rect.bounds.TopRightBack(),
				rect.bounds.TopLeftBack()
			};

			return FurthestSimplex(rect, direction, points, transforms...);
		}

		template<typename... Transforms>
		Simplex FurthestSimplex(const ShapeCapsule& capsule, const Vec3p& direction, const Transforms&... transforms)
		{
			return Simplex();
		}

		template<typename... Transforms>
		Simplex FurthestSimplex(const ShapeHull& hull, const Vec3p& direction, const Transforms&... transforms)
		{
			return Simplex();
		}

		template<typename... Transforms>
		Simplex FurthestSimplex(const ShapeMesh& mesh, const Vec3p& direction, const Transforms&... transforms)
		{
			return Simplex();
		}
	}
}