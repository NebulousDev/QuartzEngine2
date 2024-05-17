#pragma once

#include "Resource/Asset.h"
#include "VertexData.h"

namespace Quartz
{
	struct Model : public Asset
	{
		VertexData data;

		inline Model() = default;
		inline Model(File* pSourceFile) : Asset(pSourceFile) {};
	};
}