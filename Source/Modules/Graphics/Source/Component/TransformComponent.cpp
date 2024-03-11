#include "Component/TransformComponent.h"

namespace Quartz
{
    TransformComponent::TransformComponent()
        : Transform() {}

    TransformComponent::TransformComponent(const Vec3f& position, const Quatf& rotation, const Vec3f& scale) :
        Transform(position, rotation, scale)
    {
        // Nothing
    }
}

