#pragma once

#include "Collision.h"
#include "Colliders.h"
#include "Simplex.h"

#define PHYSICS_GJK_MAX_ITERATIONS	32
#define PHYSICS_EPA_MAX_ITERATIONS	32
#define PHYSICS_EPA_MAX_EDGES		256 //1536
#define PHYSICS_EPA_MAX_TRIS		64 //256
#define PHYSICS_EPA_MAX_TETRAS		64

namespace Quartz
{
	using namespace ShapeUtils;

	namespace GJK
	{
		struct Line
		{
			Vec3f points[2];

			Line();
			Line(const Vec3f& a, const Vec3f& b);
		};

		struct Triangle
		{
			Vec3f points[3];

			Triangle();
			Triangle(const Vec3f& a, const Vec3f& b, const Vec3f& c);
		};

		struct Tetrahedron
		{
			Vec3f points[4];

			Tetrahedron();
			Tetrahedron(const Vec3f& a, const Vec3f& b, const Vec3f& c, const Vec3f& d);
		};

		class Polytope
		{
		private:
			Triangle	mTris[PHYSICS_EPA_MAX_TRIS];
			uSize		mSize;

		public:
			Polytope();

			bool AddSimplex(const Simplex& simplex);
			void AddTriangle(const Triangle& tri);
			void RemoveTriangle(uSize index);
			void ClosestTriangle(const Vec3f& point, Triangle& outTri,
				uSize& outIndex, float& outDist, Vec3f& outNormal) const;
			void Extend(const Vec3f& point);
		};

		template<typename Shape0, typename Shape1>
		Vec3f MinkowskiFurthestPoint(const Shape0& shape0, const Mat4f& transform0,
			const Shape1& shape1, const Mat4f& transform1, const Vec3f direction)
		{
			const Vec3f point0 = FurthestPoint(shape0, direction, transform0);
			const Vec3f point1 = FurthestPoint(shape1, -direction, transform1);

			return point0 - point1;
		}

		template<typename Shape>
		Vec3f MinkowskiFurthestPointRect(const Shape& shape0, const Mat4f& transform0,
			const ShapeRect& rect1, Vec3f(&points)[8], const Vec3f direction)
		{
			const Vec3f point0 = FurthestPoint(shape0, direction, transform0);
			const Vec3f point1 = FurthestPoint(rect1, -direction, points);

			return point0 - point1;
		}

		inline Vec3f MinkowskiFurthestPointRectRect(const ShapeRect& rect0, Vec3f(&points0)[8],
			const ShapeRect& rect1, Vec3f(&points1)[8], const Vec3f direction)
		{
			const Vec3f point0 = FurthestPoint(rect0, direction, points0);
			const Vec3f point1 = FurthestPoint(rect1, -direction, points1);

			return point0 - point1;
		}

		template<typename Shape0, typename Shape1>
		bool GJK(const Shape0& shape0, const Mat4f& transform0, const Shape1& shape1, const Mat4f& transform1, Simplex& outSimplex)
		{
			Simplex simplex;

			// Check any inital direction (x-axis) for the furthest point in minkowski difference.
			Vec3f direction = Vec3f::X_AXIS;
			Vec3f furthestPoint = MinkowskiFurthestPoint(shape0, transform0, shape1, transform1, direction);

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
				furthestPoint = MinkowskiFurthestPoint(shape0, transform0, shape1, transform1, direction);

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

		template<typename Shape0>
		bool GJKRect(const Shape0& shape0, const Mat4f& transform0, const ShapeRect& rect1, Vec3f(&points)[8], Simplex& outSimplex)
		{
			Simplex simplex;

			// Check any inital direction (x-axis) for the furthest point in minkowski difference.
			Vec3f direction = Vec3f::X_AXIS;
			Vec3f furthestPoint = MinkowskiFurthestPointRect(shape0, transform0, rect1, points, direction);

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
				furthestPoint = MinkowskiFurthestPointRect(shape0, transform0, rect1, points, direction);

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

		inline bool GJKRectRect(const ShapeRect& rect0, Vec3f(&points0)[8], const ShapeRect& rect1, Vec3f(&points1)[8], Simplex& outSimplex)
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

		template<typename Shape0, typename Shape1>
		bool EPA(const Shape0& shape0, const Mat4f& transform0, const Shape1& shape1, const Mat4f& transform1, const Simplex& simplex, Collision& outCollision)
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
				Vec3f furthestPoint = MinkowskiFurthestPoint(shape0, transform0, shape1, transform1, normal);

				if (Dot(furthestPoint, normal) > dist + tolerance)
				{
					polytope.Extend(furthestPoint);
				}
				else
				{
					//Vec3f extent0 = FurthestPoint(collider0, normal);
					//Vec3f extent1 = extent0 - normal.Normalized() * dist;

					Simplex contact0 = FurthestSimplex(shape0, -normal, transform0);
					Simplex contact1 = FurthestSimplex(shape1, normal, transform1);

					//return Collision(normal.Normalized(), dist, contact0, contact1);
					//return Collision(); // No Collision
					return false;
				}
			}

			return false;
		}

		template<typename Shape0>
		bool EPARect(const Shape0& shape0, const Mat4f& transform0, const ShapeRect& rect1, Vec3f(&points)[8], const Simplex& simplex, Collision& outCollision)
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
				Vec3f furthestPoint = MinkowskiFurthestPointRect(shape0, transform0, rect1, points, normal);

				if (Dot(furthestPoint, normal) > dist + tolerance)
				{
					polytope.Extend(furthestPoint);
				}
				else
				{
					Simplex contact0 = FurthestSimplex(shape0, -normal, transform0);
					Simplex contact1 = FurthestSimplex(rect1, normal, points);

					Collision collision(normal.Normalized(), dist);

					if (contact1.Size() > contact0.Size())
					{
						contact0 = contact1;
					}

					for (uSize i = 0; i < contact0.Size(); i++)
					{
						collision.AddContact(contact0[i]);
					}

					outCollision = collision;

					return true;
				}
			}

			return false;
		}

		inline bool EPARectRect(const ShapeRect& rect0, Vec3f(&points0)[8], const ShapeRect& rect1, Vec3f(&points1)[8], const Simplex& simplex, Collision& outCollision)
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

					Simplex contact0 = FurthestSimplex(rect0, -normal, points0);
					Simplex contact1 = FurthestSimplex(rect1, normal, points1);

					//return Collision(normal.Normalized(), dist, contact0, contact1);
					//return Collision(); // No Collision
					return false;
				}
			}

			return false;
		}
	}
}