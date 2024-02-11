#pragma once

#include "Types/Types.h"
#include "Types/Array.h"

namespace Quartz
{
	struct ModelData
	{
		Array<float>	vertices;
		Array<uInt16>	indices;
	};
}