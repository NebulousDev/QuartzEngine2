#pragma once

#include "Math/Math.h"

namespace Quartz
{
	enum ShapeType
	{
		SHAPE_SPHERE	= 0,
		SHAPE_PLANE		= 1,
		SHAPE_RECT		= 2,
		SHAPE_CAPSULE	= 3,
		SHAPE_MESH		= 4
	};

	struct SphereShape
	{
		float radius;
	};

	struct PlaneShape
	{
		Vec3f normal;
		float length;
	};

	struct RectShape
	{
		Bounds3f bounds;
	};

	struct CapsuleShape
	{

	};

	struct MeshShape
	{

	};

	struct Collider
	{
		union
		{
			SphereShape		sphere;
			PlaneShape		plane;
			RectShape		rect;
			CapsuleShape	capsule;
			MeshShape		mesh;

			struct { char _shapeData[8 * sizeof(float)]; } shapeData;
		};

		ShapeType shape;
		Transform transform;

		inline Collider() : shapeData{} {};
		inline Collider(const Collider& collider) :
			shape(collider.shape), transform(collider.transform), shapeData(collider.shapeData) { }
	};

	class SphereCollider : public Collider
	{
	public:
		Collider::sphere;
		Collider::transform;

	public:
		inline SphereCollider(Transform transform, float radius)
		{ 
			shape = SHAPE_SPHERE;
			this->transform = transform;
			sphere.radius = radius;
		};
	};

	class PlaneCollider : public Collider
	{
	public:
		Collider::plane;
		Collider::transform;

	public:
		inline PlaneCollider(Transform transform, Vec3f normal, float length)
		{
			shape = SHAPE_PLANE;
			this->transform = transform;
			plane.normal = normal;
			plane.length = length;
		};
	};

	class RectCollider : public Collider
	{
	public:
		Collider::rect;
		Collider::transform;

	public:
		inline RectCollider(Transform transform, Bounds3f bounds)
		{
			shape = SHAPE_RECT;
			this->transform = transform;
			rect.bounds = bounds;
		};
	};

	class CapsuleCollider : public Collider
	{
	public:
		Collider::capsule;
		Collider::transform;

	public:
		inline CapsuleCollider(Transform transform)
		{
			shape = SHAPE_CAPSULE;
			this->transform = transform;
		};
	};

	class MeshCollider : protected Collider
	{
	public:
		Collider::mesh;
		Collider::transform;

	public:
		inline MeshCollider(Transform transform)
		{
			shape = SHAPE_MESH;
			this->transform = transform;
		};
	};

	struct Collision
	{
		bool isColliding;
		Vec3f extent0;
		Vec3f extent1;

		inline Collision() : extent0(), extent1(), isColliding(false) {};
		inline Collision(Vec3f extent0, Vec3f extent1, bool colliding = true) :
			extent0(extent0), extent1(extent1), isColliding(colliding) {};

		inline Collision Flip() { return Collision(extent1, extent0, isColliding); }
	};
}