#include "Colliders.h"
#include "Physics.h"

#define PHYSICS_SMALLEST_DISTANCE 0.0001f

namespace Quartz
{
	using CollideFunc = Collision (*)(Collider collider0, Collider collider1);

	Collision ResolveSphereSphere(Collider sphere0, Collider sphere1)
	{
		Vec3f diff		= sphere1.transform.position - sphere0.transform.position;
		float scale0	= 1.0f; //sphere0.transform.scale.Magnitude();
		float scale1	= 1.0f; //sphere1.transform.scale.Magnitude();
		float radius0	= scale0 * sphere0.sphere.radius;
		float radius1	= scale1 * sphere1.sphere.radius;

		float dist = diff.Magnitude();

		if (dist > PHYSICS_SMALLEST_DISTANCE && dist < (radius0 + radius1))
		{
			Vec3f normDir = diff.Normalized();
			Vec3f extent0 = sphere0.transform.position + normDir * radius0;
			Vec3f extent1 = sphere1.transform.position - normDir * radius1;

			return Collision(extent0, extent1);
		}
			
		return Collision(); // No Collision
	};

	Collision ResolveSpherePlane(Collider sphere0, Collider plane1)
	{
		float scale0	= 1.0f;//sphere0.transform.scale.Magnitude();
		float radius0	= scale0 * sphere0.sphere.radius;
		Vec3f rotNormal = plane1.transform.rotation * plane1.plane.normal;
		Vec3f planeExt	= plane1.transform.position; //rotNormal * plane1.plane.length + plane1.transform.position;

		float dist = Dot(sphere0.transform.position - planeExt, rotNormal);

		if (dist > PHYSICS_SMALLEST_DISTANCE && dist < radius0)
		{
			Vec3f extent0 = sphere0.transform.position - rotNormal * radius0;
			Vec3f extent1 = sphere0.transform.position - rotNormal * dist;

			return Collision(extent0, extent1);
		}

		return Collision(); // No Collision
	}

	Collision ResolveSphereRect(Collider sphere0, Collider rect1)
	{
		return Collision(); // No Collision
	}

	Collision ResolveSphereCapsule(Collider sphere0, Collider capsule1)
	{
		return Collision(); // No Collision
	}

	Collision ResolveSphereMesh(Collider sphere0, Collider mesh1)
	{
		return Collision(); // No Collision
	}

	Collision ResolvePlaneSphere(Collider plane0, Collider sphere1) { return ResolveSpherePlane(sphere1, plane0).Flip(); };

	Collision ResolvePlanePlane(Collider plane0, Collider plane1)
	{
		return Collision(); // No Collision
	}

	Collision ResolvePlaneRect(Collider plane0, Collider rect1)
	{
		return Collision(); // No Collision
	}

	Collision ResolvePlaneCapsule(Collider plane0, Collider capsule1)
	{
		return Collision(); // No Collision
	}

	Collision ResolvePlaneMesh(Collider plane0, Collider mesh1)

	{
		return Collision(); // No Collision
	}

	Collision ResolveRectSphere(Collider rect0, Collider sphere1) { return ResolveSphereRect(sphere1, rect0).Flip(); };
	Collision ResolveRectPlane(Collider rect0, Collider plane1) { return ResolvePlaneRect(plane1, rect0).Flip(); };

	Collision ResolveRectRect(Collider rect0, Collider rect1)
	{
		return Collision(); // No Collision
	}

	Collision ResolveRectCapsule(Collider rect0, Collider capsule1)
	{
		return Collision(); // No Collision
	}

	Collision ResolveRectMesh(Collider rect0, Collider mesh1)
	{
		return Collision(); // No Collision
	}

	Collision ResolveCapsuleSphere(Collider capsule0, Collider sphere1) { return ResolveSphereCapsule(sphere1, capsule0).Flip(); };
	Collision ResolveCapsulePlane(Collider capsule0, Collider plane1) { return ResolvePlaneCapsule(plane1, capsule0).Flip(); };
	Collision ResolveCapsuleRect(Collider capsule0, Collider rect1) { return ResolveRectCapsule(rect1, capsule0).Flip(); };

	Collision ResolveCapsuleCapsule(Collider capsule0, Collider capsule1)
	{
		return Collision(); // No Collision
	}

	Collision ResolveCapsuleMesh(Collider capsule0, Collider mesh1)
	{
		return Collision(); // No Collision
	}

	Collision ResolveMeshSphere(Collider mesh0, Collider sphere1) { return ResolveSphereMesh(sphere1, mesh0).Flip(); };
	Collision ResolveMeshPlane(Collider mesh0, Collider plane1) { return ResolvePlaneMesh(plane1, mesh0).Flip(); };
	Collision ResolveMeshRect(Collider mesh0, Collider rect1) { return ResolveRectMesh(rect1, mesh0).Flip(); };
	Collision ResolveMeshCapsule(Collider mesh0, Collider capsule1) { return ResolveCapsuleMesh(capsule1, mesh0).Flip(); };

	Collision ResolveMeshMesh(Collider mesh0, Collider mesh1)
	{
		return Collision(); // No Collision
	}

	Collision ResolveCollisionOld(Collider collider0, Collider collider1)
	{
		switch (collider0.shape)
		{
			case SHAPE_SPHERE:
			{
				switch (collider1.shape)
				{
					case SHAPE_SPHERE:
					{
						return ResolveSphereSphere(collider0, collider1);
					}
			
					case SHAPE_PLANE:
					{
						return ResolveSpherePlane(collider0, collider1);
					}

					case SHAPE_RECT:
					{
						return ResolveSphereRect(collider0, collider1);
					}

					case SHAPE_CAPSULE:
					{
						return ResolveSphereCapsule(collider0, collider1);
					}

					case SHAPE_MESH:
					{
						return ResolveSphereMesh(collider0, collider1);
					}
				}

				break;
			}
			
			case SHAPE_PLANE:
			{
				switch (collider1.shape)
				{
					case SHAPE_SPHERE:
					{
						return ResolveSpherePlane(collider1, collider0);
					}
			
					case SHAPE_PLANE:
					{
						return ResolvePlanePlane(collider0, collider1);
					}

					case SHAPE_RECT:
					{
						return ResolvePlaneRect(collider0, collider1);
					}

					case SHAPE_CAPSULE:
					{
						return ResolvePlaneCapsule(collider0, collider1);
					}

					case SHAPE_MESH:
					{
						return ResolvePlaneMesh(collider0, collider1);
					}
				}

				break;
			}

			case SHAPE_RECT:
			{
				switch (collider1.shape)
				{
					case SHAPE_SPHERE:
					{
						return ResolveSphereRect(collider1, collider0);
					}
			
					case SHAPE_PLANE:
					{
						return ResolvePlaneRect(collider1, collider0);
					}

					case SHAPE_RECT:
					{
						return ResolveRectRect(collider0, collider1);
					}

					case SHAPE_CAPSULE:
					{
						return ResolveRectCapsule(collider0, collider1);
					}

					case SHAPE_MESH:
					{
						return ResolveRectMesh(collider0, collider1);
					}
				}

				break;
			}

			case SHAPE_CAPSULE:
			{
				switch (collider1.shape)
				{
					case SHAPE_SPHERE:
					{
						return ResolveSphereCapsule(collider1, collider0);
					}
			
					case SHAPE_PLANE:
					{
						return ResolvePlaneCapsule(collider1, collider0);
					}

					case SHAPE_RECT:
					{
						return ResolveRectCapsule(collider1, collider0);
					}

					case SHAPE_CAPSULE:
					{
						return ResolveCapsuleCapsule(collider0, collider1);
					}

					case SHAPE_MESH:
					{
						return ResolveCapsuleMesh(collider0, collider1);
					}
				}

				break;
			}

			case SHAPE_MESH:
			{
				switch (collider1.shape)
				{
					case SHAPE_SPHERE:
					{
						return ResolveSphereMesh(collider1, collider0);
					}
			
					case SHAPE_PLANE:
					{
						return ResolvePlaneMesh(collider1, collider0);
					}

					case SHAPE_RECT:
					{
						return ResolveRectMesh(collider1, collider0);
					}

					case SHAPE_CAPSULE:
					{
						return ResolveCapsuleMesh(collider1, collider0);
					}

					case SHAPE_MESH:
					{
						return ResolveMeshMesh(collider0, collider1);
					}
				}

				break;
			}
		}
	}

	Collision Physics::ResolveCollision(Collider collider0, Collider collider1)
	{
		static CollideFunc functionTable[25]
		{
			ResolveSphereSphere,
			ResolveSpherePlane,
			ResolveSphereRect,
			ResolveSphereCapsule,
			ResolveSphereMesh,

			ResolvePlaneSphere,
			ResolvePlanePlane,
			ResolvePlaneRect,
			ResolvePlaneCapsule,
			ResolvePlaneMesh,

			ResolveRectSphere,
			ResolveRectPlane,
			ResolveRectRect,
			ResolveRectCapsule,
			ResolveRectMesh,

			ResolveCapsuleSphere,
			ResolveCapsulePlane,
			ResolveCapsuleRect,
			ResolveCapsuleCapsule,
			ResolveCapsuleMesh,

			ResolveMeshSphere,
			ResolveMeshPlane,
			ResolveMeshRect,
			ResolveMeshCapsule,
			ResolveMeshMesh
		};

		uSize index = (uSize)collider1.shape + ((uSize)collider0.shape * 5);

		return functionTable[index](collider0, collider1);
	}

}