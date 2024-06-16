#pragma once

#include "EngineAPI.h"
#include "Entity/Component.h"
#include "Resource/Assets/Model.h"

namespace Quartz
{
	struct QUARTZ_ENGINE_API MeshComponent : public Component<MeshComponent>
	{
		String modelURI;
		uInt64 modelURIHash;
		Model* pCachedModel;

		MeshComponent();
		MeshComponent(const String& uri);
	};
}