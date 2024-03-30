#pragma once

#include "PhysicsTypes.h"
#include "Math/Math.h"

namespace Quartz
{
	struct RigidBody
	{
		floatp invMass;
		floatp restitution;	 // bouncy-ness
		floatp friction;
		Vec3p gravity;

		Vec3p force;
		Vec3p torque;
		Vec3p linearVelocity;
		Vec3p angularVelocity;
		Vec3p inertiaVector;
		Mat3p invInertiaTensor;
		bool  asleep;

		Vec3p lastAcceleration;

		inline RigidBody() :
			invMass(1.0f),
			restitution(0.5f),
			friction(0.5f),
			gravity(0.0f, -9.81f, 0.0f) {}

		inline RigidBody(floatp invMass, floatp restitution, floatp friction,
			const Vec3p& gravity = { 0.0f, -9.81f, 0.0f }) :
			invMass(invMass),
			restitution(restitution),
			friction(friction),
			gravity(gravity) {}

		inline void AddForce(const Vec3p& force)
		{
			this->force += force;
		}

		inline void AddTorque(const Vec3p& torque)
		{
			this->torque += torque;
		}

		inline void AddLinearVelocity(const Vec3p& velocity)
		{
			this->linearVelocity += velocity;
		}

		inline void AddAngularVelocity(const Vec3p& velocity)
		{
			this->angularVelocity += velocity;
		}

        static inline void _calculateTransformMatrix(Mat4p& transformMatrix,
            const Vec3p& position,
            const Quatp& orientation)
        {
            transformMatrix.e[0] = 1 - 2 * orientation.y * orientation.y -
                2 * orientation.z * orientation.z;
            transformMatrix.e[1] = 2 * orientation.x * orientation.y -
                2 * orientation.w * orientation.z;
            transformMatrix.e[2] = 2 * orientation.x * orientation.z +
                2 * orientation.w * orientation.y;
            transformMatrix.e[3] = position.x;

            transformMatrix.e[4] = 2 * orientation.x * orientation.y +
                2 * orientation.w * orientation.z;
            transformMatrix.e[5] = 1 - 2 * orientation.x * orientation.x -
                2 * orientation.z * orientation.z;
            transformMatrix.e[6] = 2 * orientation.y * orientation.z -
                2 * orientation.w * orientation.x;
            transformMatrix.e[7] = position.y;

            transformMatrix.e[8] = 2 * orientation.x * orientation.z -
                2 * orientation.w * orientation.y;
            transformMatrix.e[9] = 2 * orientation.y * orientation.z +
                2 * orientation.w * orientation.x;
            transformMatrix.e[10] = 1 - 2 * orientation.x * orientation.x -
                2 * orientation.y * orientation.y;
            transformMatrix.e[11] = position.z;
        }

        static inline void _transformInertiaTensor(Mat3p& iitWorld,
            const Mat3p& iitBody,
            const Mat4p& rotmat)
        {
            floatp t4 = rotmat.e[0] * iitBody.e[0] +
                rotmat.e[1] * iitBody.e[3] +
                rotmat.e[2] * iitBody.e[6];
            floatp t9 = rotmat.e[0] * iitBody.e[1] +
                rotmat.e[1] * iitBody.e[4] +
                rotmat.e[2] * iitBody.e[7];
            floatp t14 = rotmat.e[0] * iitBody.e[2] +
                rotmat.e[1] * iitBody.e[5] +
                rotmat.e[2] * iitBody.e[8];
            floatp t28 = rotmat.e[4] * iitBody.e[0] +
                rotmat.e[5] * iitBody.e[3] +
                rotmat.e[6] * iitBody.e[6];
            floatp t33 = rotmat.e[4] * iitBody.e[1] +
                rotmat.e[5] * iitBody.e[4] +
                rotmat.e[6] * iitBody.e[7];
            floatp t38 = rotmat.e[4] * iitBody.e[2] +
                rotmat.e[5] * iitBody.e[5] +
                rotmat.e[6] * iitBody.e[8];
            floatp t52 = rotmat.e[8] * iitBody.e[0] +
                rotmat.e[9] * iitBody.e[3] +
                rotmat.e[10] * iitBody.e[6];
            floatp t57 = rotmat.e[8] * iitBody.e[1] +
                rotmat.e[9] * iitBody.e[4] +
                rotmat.e[10] * iitBody.e[7];
            floatp t62 = rotmat.e[8] * iitBody.e[2] +
                rotmat.e[9] * iitBody.e[5] +
                rotmat.e[10] * iitBody.e[8];

            iitWorld.e[0] = t4 * rotmat.e[0] +
                t9 * rotmat.e[1] +
                t14 * rotmat.e[2];
            iitWorld.e[1] = t4 * rotmat.e[4] +
                t9 * rotmat.e[5] +
                t14 * rotmat.e[6];
            iitWorld.e[2] = t4 * rotmat.e[8] +
                t9 * rotmat.e[9] +
                t14 * rotmat.e[10];
            iitWorld.e[3] = t28 * rotmat.e[0] +
                t33 * rotmat.e[1] +
                t38 * rotmat.e[2];
            iitWorld.e[4] = t28 * rotmat.e[4] +
                t33 * rotmat.e[5] +
                t38 * rotmat.e[6];
            iitWorld.e[5] = t28 * rotmat.e[8] +
                t33 * rotmat.e[9] +
                t38 * rotmat.e[10];
            iitWorld.e[6] = t52 * rotmat.e[0] +
                t57 * rotmat.e[1] +
                t62 * rotmat.e[2];
            iitWorld.e[7] = t52 * rotmat.e[4] +
                t57 * rotmat.e[5] +
                t62 * rotmat.e[6];
            iitWorld.e[8] = t52 * rotmat.e[8] +
                t57 * rotmat.e[9] +
                t62 * rotmat.e[10];
        }

		inline void UpdateInertia(const Transform& transform)
		{
			if (!inertiaVector.IsZero())
			{
				//invInertiaTensor = (Mat3p().SetRotation(transform.rotation) * Mat3p().SetIdentity(inertiaVector)).Inverse();
                
                Mat4p rotMat;
                _calculateTransformMatrix(rotMat, transform.position, transform.rotation);
                //Mat4p rotMat(transform.GetMatrix());
                _transformInertiaTensor(invInertiaTensor, Mat3p().SetIdentity(inertiaVector), rotMat);

                //invInertiaTensor = Mat3p().SetIdentity();

                //invInertiaTensor = transform.GetMatrix() * Mat3p().SetIdentity(inertiaVector).Inverse();
			}
		}
	};
}