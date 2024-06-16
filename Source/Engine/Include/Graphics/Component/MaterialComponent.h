#pragma once

#include "EngineAPI.h"
#include "Entity/Component.h"
#include "Resource/Assets/Material.h"

#include "Types/String.h"

namespace Quartz
{
	struct QUARTZ_ENGINE_API MaterialComponent : public Component<MaterialComponent>
	{
		Array<String> materialPaths;
		Array<Material*> pCachedMaterials;

		MaterialComponent() = default;
		MaterialComponent(const Array<String>& materialPaths);
	};
}