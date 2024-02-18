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

    Vec3f TransformComponent::GetForward()
    {
        return rotation * Vec3f( 0.0f, 0.0f, 1.0f );
    }

    Vec3f TransformComponent::GetBackward()
    {
        return rotation * Vec3f( 0.0f, 0.0f, -1.0f );
    }

    Vec3f TransformComponent::GetLeft()
    {
        return rotation * Vec3f( 1.0f, 0.0f, 0.0f );
    }

    Vec3f TransformComponent::GetRight()
    {
        return rotation * Vec3f( -1.0f, 0.0f, 0.0f );
    }

    Vec3f TransformComponent::GetUp()
    {
        return rotation * Vec3f( 0.0f, 1.0f, 1.0f );
    }

    Vec3f TransformComponent::GetDown()
    {
        return rotation * Vec3f( 0.0f, -1.0f, 1.0f );
    }

    Mat4f TransformComponent::GetMatrix()
    {
#if 0
        return 
            Mat4f().SetTranslation(position) *
            Mat4f().SetRotation(rotation) *
            Mat4f().SetScale(scale);
#else
        return
            (Mat4f().SetScale(scale) *
            Mat4f().SetRotation(rotation)) *
            Mat4f().SetTranslation(position);
#endif
    }

    Mat4f TransformComponent::GetViewMatrix()
    {       
        return
            Mat4f().SetTranslation(position) *
            Mat4f().SetRotation(rotation) *
            Mat4f().SetScale(scale);

    }
}

