#include "Component/MeshComponent.h"

namespace Quartz
{
	MeshComponent::MeshComponent() :
		modelURI(), modelURIHash(0)
	{
		modelData = {};
	}

	MeshComponent::MeshComponent(const String& uri, const ModelData& modelData) :
		modelURI(uri), modelURIHash(0), modelData(modelData)
	{
		modelURIHash = Hash(uri);
	}
}