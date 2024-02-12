#pragma once

#include "GfxAPI.h"
#include "Entity/Component.h"

#include "Types/String.h"

namespace Quartz
{
	struct QUARTZ_GRAPHICS_API MaterialComponent : public Component<MaterialComponent>
	{
		String vertexURI;
		String fragmentURI;

		uInt64 vertexURIHash;
		uInt64 fragmentURIHash;

		MaterialComponent();
		MaterialComponent(const String& vertexURI, const String& fragmentURI);
	};
}