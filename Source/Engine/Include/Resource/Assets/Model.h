#pragma once

#include "Resource/Asset.h"

#include "Material.h"
#include "VertexData.h"

namespace Quartz
{
	struct Model : public Asset
	{
		VertexData	data;
		//Material	material;
	};
}