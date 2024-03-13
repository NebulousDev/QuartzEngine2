#include "Physics.h"

namespace Quartz
{
	using FurthestPointFunc = Vec3f(*)(const Collider& collider, const Vec3f& direction);

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
		Vec3f maxPoint;

		Vec3f normDir = direction.Normalized();

		for (uSize i = 0; i < 8; i++)
		{
			float dist = Dot(points[i], normDir);

			if (dist > maxDist)
			{
				maxDist = dist;
				maxPoint = points[i];
			}
		}

		return maxPoint;
	}

	Vec3f Physics::FurthestPointRect(const Collider& rect, const Vec3f& direction)
	{
		Mat4f rectTransform = rect.transform.GetMatrix();
		Bounds3f rectBounds = rect.rect.bounds;

		// Transform Rect in to world space
		//rectBounds.start = rectTransform * rectBounds.start;
		//rectBounds.end = rectTransform * rectBounds.end;

		Vec3f points[8]
		{
			rectTransform * rectBounds.BottomRightFront(),
			rectTransform * rectBounds.BottomLeftFront(),
			rectTransform * rectBounds.BottomRightBack(),
			rectTransform * rectBounds.BottomLeftBack(),
			rectTransform * rectBounds.TopRightFront(),
			rectTransform * rectBounds.TopLeftFront(),
			rectTransform * rectBounds.TopRightBack(),
			rectTransform * rectBounds.TopLeftBack()
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
}