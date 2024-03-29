#pragma once

#include "Math/Math.h"

#define PHYSICS_USE_DOUBLE 0

namespace Quartz
{
#if PHYSICS_USE_DOUBLE
	using floatP = double;
	using Vec3P = Vec3d;
	using Mat3P = Mat3d;
#else
	using floatP = float;
	using Vec3P = Vec3f;
	using Mat3P = Mat3f;
#endif
}