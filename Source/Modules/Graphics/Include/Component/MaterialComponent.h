#pragma once

#include "GfxAPI.h"
#include "Entity/Component.h"
#include "Resource/Assets/Material.h"

#include "Types/String.h"

namespace Quartz
{
	struct QUARTZ_GRAPHICS_API MaterialComponent : public Component<MaterialComponent>
	{
		Array<String> materialPaths;
		Array<Material*> pCachedMaterials;

		MaterialComponent() = default;
		MaterialComponent(const Array<String>& materialPaths);
	};
}