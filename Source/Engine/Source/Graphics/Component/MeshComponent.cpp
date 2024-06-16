#include "Graphics/Component/MeshComponent.h"

namespace Quartz
{
	MeshComponent::MeshComponent() :
		modelURI(), modelURIHash(0), pCachedModel(nullptr) { }

	MeshComponent::MeshComponent(const String& uri) :
		modelURI(uri), modelURIHash(0), pCachedModel(nullptr)
	{
		modelURIHash = Hash(uri);
	}
}