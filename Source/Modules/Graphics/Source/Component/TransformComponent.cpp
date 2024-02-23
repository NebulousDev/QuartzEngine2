#include "Component/TransformComponent.h"

namespace Quartz
{
    TransformComponent::TransformComponent() :
        position({ 0.0f, 0.0f, 0.0f }),
        rotation(Quatf().SetEuler({ 0.0f, 0.0f, 0.0f })),
        scale({ 1.0f, 1.0f, 1.0f })
    {
        // Nothing
    }

    TransformComponent::TransformComponent(const Vec3f& position, const Quatf& rotation, const Vec3f& scale) :
        position(position),
        rotation(rotation),
        scale(scale)
    {
        // Nothing
    }

    Vec3f TransformComponent::GetForward() const
    {
        return rotation * Vec3f( 0.0f, 0.0f, 1.0f );
    }

    Vec3f TransformComponent::GetBackward() const
    {
        return rotation * Vec3f( 0.0f, 0.0f, -1.0f );
    }

    Vec3f TransformComponent::GetLeft() const
    {
        return rotation * Vec3f( -1.0f, 0.0f, 0.0f );
    }

    Vec3f TransformComponent::GetRight() const
    {
        return rotation * Vec3f( 1.0f, 0.0f, 0.0f );
    }

    Vec3f TransformComponent::GetUp() const
    {
        return rotation * Vec3f( 0.0f, 1.0f, 1.0f );
    }

    Vec3f TransformComponent::GetDown() const
    {
        return rotation * Vec3f( 0.0f, -1.0f, 1.0f );
    }

    Mat4f TransformComponent::GetMatrix() const
    {
        return
            Mat4f().SetScale(scale) *
            Mat4f().SetRotation(rotation) *
            Mat4f().SetTranslation(position);
    }

    Mat4f TransformComponent::GetViewMatrix() const
    {       
        return
            Mat4f().SetTranslation(position) *
            Mat4f().SetRotation(rotation);

    }
}

