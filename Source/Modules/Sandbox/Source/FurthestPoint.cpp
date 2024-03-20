#include "Physics.h"

namespace Quartz
{
	using FurthestPointFunc = Vec3f(*)(const Collider& collider, const Vec3f& direction);
	using FurthestSimplexFunc = Simplex(*)(const Collider& collider, const Vec3f& direction);

	Vec3f Physics::FurthestPointSphere(const Collider& sphere, const Vec3f& direction)
	{
		Vec3f normDir = direction.Normalized();
		float radius = sphere.sphere.radius * sphere.transform.scale.Maximum();
		return normDir * radius + sphere.transform.position;
	}

	Vec3f Physics::FurthestPointPlane(const Collider& plane, const Vec3f& direction)
	{
		return {};
	}

	Vec3f Physics::FurthestPointRect(const Collider& rect, const Vec3f& direction, Vec3f (& points)[8])
	{
		float maxDist = -FLT_MAX;
		uSize maxPointIndex = 0;

		Vec3f normDir = direction.Normalized();

		for (uSize i = 0; i < 8; i++)
		{
			float dist = Dot(points[i], normDir);

			if (dist > maxDist)
			{
				maxDist = dist;
				maxPointIndex = i;
			}
		}

		return points[maxPointIndex];
	}

	Vec3f Physics::FurthestPointRect(const Collider& rect, const Vec3f& direction)
	{
		Mat4f rectTransform = rect.transform.GetMatrix();

		Vec3f points[8]
		{
			rectTransform * rect.rect.bounds.BottomRightFront(),
			rectTransform * rect.rect.bounds.BottomLeftFront(),
			rectTransform * rect.rect.bounds.BottomRightBack(),
			rectTransform * rect.rect.bounds.BottomLeftBack(),
			rectTransform * rect.rect.bounds.TopRightFront(),
			rectTransform * rect.rect.bounds.TopLeftFront(),
			rectTransform * rect.rect.bounds.TopRightBack(),
			rectTransform * rect.rect.bounds.TopLeftBack()
		};

		return FurthestPointRect(rect, direction, points);
	}

	Vec3f Physics::FurthestPointCapsule(const Collider& capsule, const Vec3f& direction)
	{
		return {};
	}

	Vec3f Physics::FurthestPointHull(const Collider& hull, const Vec3f& direction)
	{
		return {};
	}

	Vec3f Physics::FurthestPointMesh(const Collider& mesh, const Vec3f& direction)
	{
		return {};
	}

	Vec3f Physics::FurthestPoint(const Collider& collider0, const Vec3f& direction)
	{
		static FurthestPointFunc functionTable[6]
		{
			FurthestPointSphere,
			FurthestPointPlane,
			FurthestPointRect,
			FurthestPointCapsule,
			FurthestPointHull,
			FurthestPointMesh
		};

		return functionTable[(uSize)collider0.shape](collider0, direction);
	}

	Simplex Physics::FurthestSimplexSphere(const Collider& collider0, const Vec3f& direction)
	{
		Simplex simplex;
		simplex.Push(collider0.transform.position + direction.Normalized() * collider0.sphere.radius);
		return simplex;
	}

	Simplex Physics::FurthestSimplexPlane(const Collider& plane, const Vec3f& direction)
	{
		return Simplex();
	}

	Simplex Physics::FurthestSimplexRect(const Collider& rect, const Vec3f& direction, Vec3f(&points)[8])
	{
		Simplex simplex;

		float maxDist0 = -FLT_MAX;
		float maxDist1 = -FLT_MAX;
		float maxDist2 = -FLT_MAX;

		uSize maxPointIndex0 = {};
		uSize maxPointIndex1 = {};
		uSize maxPointIndex2 = {};

		Vec3f normDir = direction.Normalized();

		for (uSize i = 0; i < 8; i++)
		{
			float dist = Dot(points[i], normDir);

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

	Simplex Physics::FurthestSimplexRect(const Collider& rect, const Vec3f& direction)
	{
		Mat4f rectTransform = rect.transform.GetMatrix();

		Vec3f points[8]
		{
			rectTransform * rect.rect.bounds.BottomRightFront(),
			rectTransform * rect.rect.bounds.BottomLeftFront(),
			rectTransform * rect.rect.bounds.BottomRightBack(),
			rectTransform * rect.rect.bounds.BottomLeftBack(),
			rectTransform * rect.rect.bounds.TopRightFront(),
			rectTransform * rect.rect.bounds.TopLeftFront(),
			rectTransform * rect.rect.bounds.TopRightBack(),
			rectTransform * rect.rect.bounds.TopLeftBack()
		};

		return FurthestSimplexRect(rect, direction, points);
	}

	Simplex Physics::FurthestSimplexCapsule(const Collider& capsule, const Vec3f& direction)
	{
		return Simplex();
	}

	Simplex Physics::FurthestSimplexHull(const Collider& hull, const Vec3f& direction)
	{
		return Simplex();
	}

	Simplex Physics::FurthestSimplexMesh(const Collider& mesh, const Vec3f& direction)
	{
		return Simplex();
	}

	Simplex Physics::FurthestSimplex(const Collider& collider0, const Vec3f& direction)
	{
		static FurthestSimplexFunc functionTable[6]
		{
			FurthestSimplexSphere,
			FurthestSimplexPlane,
			FurthestSimplexRect,
			FurthestSimplexCapsule,
			FurthestSimplexHull,
			FurthestSimplexMesh
		};

		return functionTable[(uSize)collider0.shape](collider0, direction);
	}
}