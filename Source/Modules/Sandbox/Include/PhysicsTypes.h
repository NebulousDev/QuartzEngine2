#pragma once

#include "Math/Math.h"

#define PHYSICS_USE_DOUBLE 1

namespace Quartz
{
#if PHYSICS_USE_DOUBLE
	using floatp = double;
	using Vec3p = Vec3d;
	using Mat3p = Mat3d;
	using Mat4p = Mat4d;
	using Quatp = Quatd;
#else
	using floatp = float;
	using Vec3p = Vec3f;
	using Mat3p = Mat3f;
	using Mat4p = Mat4f;
	using Quatp = Quatf;
#endif
}