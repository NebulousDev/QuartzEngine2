#pragma once

#include "Math/Math.h"
#include "Shapes.h"

namespace Quartz
{
	class Collider
	{
	protected:
		union
		{
			ShapeSphere		sphere;
			ShapePlane		plane;
			ShapeRect		rect;
			ShapeCapsule	capsule;
			ShapeHull		hull;
			ShapeMesh		mesh;

			struct { char _shapeData[8 * sizeof(float)]; } shapeData;
		};

		ShapeType	shape;
		bool		isStatic;

	public:
		inline Collider() : 
			shape(SHAPE_NONE), shapeData{} {};

		inline Collider(const Collider& collider) :
			shape(collider.shape), shapeData(collider.shapeData), isStatic(collider.isStatic) { }

		inline void SetStatic(bool isStatic) { this->isStatic = isStatic; }

		template<typename ShapeType>
		inline const ShapeType& GetShape() const { return (ShapeType)shapeData; }

		inline const ShapeType& GetShapeType() const { return shape; }
		inline bool				IsStatic() const { return isStatic; }
	};

	class SphereCollider : public Collider
	{
	public:
		inline SphereCollider(float radius, bool isStatic = false)
		{ 
			this->shape = SHAPE_SPHERE;
			this->sphere.radius = radius;
			this->isStatic = isStatic;
		};

		inline const ShapeSphere& GetSphere() const { return sphere; }
	};

	class PlaneCollider : public Collider
	{
	public:
		inline PlaneCollider(Vec3f normal, float length, bool isStatic = false)
		{
			this->shape = SHAPE_PLANE;
			this->plane.normal = normal;
			this->plane.length = length;
			this->isStatic = isStatic;
		};

		inline const ShapePlane& GetPlane() const { return plane; }
	};

	class RectCollider : public Collider
	{
	public:
		inline RectCollider(Bounds3f bounds, bool isStatic = false)
		{
			this->shape = SHAPE_RECT;
			this->rect.bounds = bounds;
			this->isStatic = isStatic;
		};

		inline const ShapeRect& GetRect() const { return rect; }
	};

	class CapsuleCollider : public Collider
	{
	public:
		inline CapsuleCollider(bool isStatic = false)
		{
			this->shape = SHAPE_CAPSULE;
			this->isStatic = isStatic;
		};

		inline const ShapeCapsule& GetCapsule() const { return capsule; }
	};

	class HullCollider : public Collider
	{
	public:
		inline HullCollider(bool isStatic = false)
		{
			this->shape = SHAPE_HULL;
			this->isStatic = isStatic;
		};

		inline const ShapeHull& GetHull() const { return hull; }
	};

	class MeshCollider : public Collider
	{
	public:
		inline MeshCollider(bool isStatic = false)
		{
			this->shape = SHAPE_MESH;
			this->isStatic = isStatic;
		};

		inline const ShapeMesh& GetMesh() const { return mesh; }
	};
}