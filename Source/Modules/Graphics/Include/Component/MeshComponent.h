#pragma once

#include "GfxAPI.h"
#include "../ModelData.h"
#include "Entity/Component.h"

namespace Quartz
{
	struct QUARTZ_GRAPHICS_API MeshComponent : public Component<MeshComponent>
	{
		String		modelURI;
		uInt64		modelURIHash;
		ModelData	modelData;

		MeshComponent() = default;
		MeshComponent(const String& uri, const ModelData& modelData);
	};
}