#include "Component/MaterialComponent.h"

namespace Quartz
{
	MaterialComponent::MaterialComponent() :
		vertexURI(), fragmentURI(), vertexURIHash(0), fragmentURIHash(0)
	{
		// Nothing
	}

	MaterialComponent::MaterialComponent(const String& vertexURI, const String& fragmentURI) :
		vertexURI(vertexURI), fragmentURI(fragmentURI)
	{
		vertexURIHash = Hash(vertexURI);
		fragmentURIHash = Hash(fragmentURI);
	}
}
