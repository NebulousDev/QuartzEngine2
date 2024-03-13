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
		SHAPE_HULL		= 4,
		SHAPE_MESH		= 5
	};

	struct SphereShape
	{
		float radius;
		// center
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

	struct HullShape
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
			HullShape		hull;
			MeshShape		mesh;

			struct { char _shapeData[8 * sizeof(float)]; } shapeData;
		};

		ShapeType	shape;
		Transform	transform;
		bool		isStatic;

		inline Collider() : shapeData{} {};
		inline Collider(const Collider& collider) :
			shape(collider.shape), transform(collider.transform), 
			shapeData(collider.shapeData), isStatic(collider.isStatic) { }
	};

	class SphereCollider : public Collider
	{
	public:
		Collider::sphere;
		Collider::transform;

	public:
		inline SphereCollider(Transform transform, float radius, bool isStatic = false)
		{ 
			this->shape = SHAPE_SPHERE;
			this->transform = transform;
			this->sphere.radius = radius;
			this->isStatic = isStatic;
		};
	};

	class PlaneCollider : public Collider
	{
	public:
		Collider::plane;
		Collider::transform;

	public:
		inline PlaneCollider(Transform transform, Vec3f normal, float length, bool isStatic = false)
		{
			this->shape = SHAPE_PLANE;
			this->transform = transform;
			this->plane.normal = normal;
			this->plane.length = length;
			this->isStatic = isStatic;
		};
	};

	class RectCollider : public Collider
	{
	public:
		Collider::rect;
		Collider::transform;

	public:
		inline RectCollider(Transform transform, Bounds3f bounds, bool isStatic = false)
		{
			this->shape = SHAPE_RECT;
			this->transform = transform;
			this->rect.bounds = bounds;
			this->isStatic = isStatic;
		};
	};

	class CapsuleCollider : public Collider
	{
	public:
		Collider::capsule;
		Collider::transform;

	public:
		inline CapsuleCollider(Transform transform, bool isStatic = false)
		{
			this->shape = SHAPE_CAPSULE;
			this->transform = transform;
			this->isStatic = isStatic;
		};
	};

	class HullCollider : protected Collider
	{
	public:
		Collider::hull;
		Collider::transform;

	public:
		inline HullCollider(Transform transform, bool isStatic = false)
		{
			this->shape = SHAPE_HULL;
			this->transform = transform;
			this->isStatic = isStatic;
		};
	};

	class MeshCollider : protected Collider
	{
	public:
		Collider::mesh;
		Collider::transform;

	public:
		inline MeshCollider(Transform transform, bool isStatic = false)
		{
			this->shape = SHAPE_MESH;
			this->transform = transform;
			this->isStatic = isStatic;
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