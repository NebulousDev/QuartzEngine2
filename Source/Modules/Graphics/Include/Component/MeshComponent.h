#pragma once

#include "GfxAPI.h"
#include "Entity/Component.h"
#include "Resource/Assets/Model.h"

namespace Quartz
{
	struct QUARTZ_GRAPHICS_API MeshComponent : public Component<MeshComponent>
	{
		String modelURI;
		uInt64 modelURIHash;
		Model* pCachedModel;

		MeshComponent();
		MeshComponent(const String& uri);
	};
}