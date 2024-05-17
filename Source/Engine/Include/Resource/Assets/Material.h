#pragma once

#include "Resource/Asset.h"
#include "Resource/Assets/Image.h"

namespace Quartz
{
	struct Material : public Asset
	{
		Array<Image*, 8> textures;
		Array<String, 8> shaderPaths;
	};
}