#include "Entity/EntityGraph.h"

#include "Entity/EntityDatabase.h"

namespace Quartz
{
    EntityGraphView::EntityGraphView(EntityGraph* pParentGraph, SceneNode* pRoot)
        : mpParentGraph(pParentGraph),
        mpViewRoot(pRoot)
    {
        // Nothing
    }

    SceneNode* EntityGraphView::GetNode(Entity entity)
    {
        // TODO: Add check here to ensure the entity is within the view?
        return mpParentGraph->GetNode(entity);
    }

    void EntityGraph::SetDatabase(EntityDatabase* pWorld)
    {
        this->mpDatabase = pWorld;
    }

    // Should never be called on root
    void EntityGraph::UpdateBranch(SceneNode* node)
    {
        node->globalTransform = node->pLocalTransform->GetMatrix() * node->pParent->globalTransform;

        for (SceneNode* pNode : node->children)
        {
            UpdateBranch(pNode);
        }
    }

    EntityGraph::EntityGraph()
        : EntityGraph(nullptr)
    {
       // Nothing
    }

    EntityGraph::EntityGraph(EntityDatabase* mpDatabase)
        : mpDatabase(mpDatabase)
    {
        SceneNode rootNode;
        rootNode.pParent = nullptr;
        rootNode.entity = NullEntity;

        mZeroTransform = TransformComponentOld(
            { 0.0f, 0.0f, 0.0f },
            { 0.0f, 0.0f, 0.0f, 0.0f },
            { 1.0f, 1.0f, 1.0f });

        rootNode.pLocalTransform = &mZeroTransform;
        rootNode.globalTransform = Mat4f().SetIdentity();

        mpRoot = &mNodes.Insert(NullEntity, rootNode);
    }

    SceneNode* EntityGraph::GetNode(Entity entity)
    {
        if (mNodes.Contains(entity))
        {
            return &mNodes.Get(entity);
        }

        return nullptr;
    }

    // @TODO: This needs to be remade
    bool EntityGraph::ParentEntity(Entity entity, Entity parent)
    {
        // TODO: check for cycles

        if (entity == parent)
        {
            // Cant parent itself
            return false;
        }

        if (!mNodes.Contains(parent))
        {
            // Parent must be present
            return false;
        }

        SceneNode& parentNode = mNodes.Get(parent); // @TODO: Fails when mNodes is resized
        SceneNode* pChildNode = nullptr;

        if (!mNodes.Contains(entity))
        {
            SceneNode entityNode;
            entityNode.pParent = &parentNode;
            entityNode.entity = entity;

            if (mpDatabase->HasComponent<TransformComponentOld>(entity))
            {
                TransformComponentOld& transform = mpDatabase->GetComponent<TransformComponentOld>(entity);
                entityNode.pLocalTransform = &transform;
                entityNode.globalTransform = parentNode.globalTransform * transform.GetMatrix();
            }
            else
            {
                entityNode.pLocalTransform = &mZeroTransform;
                entityNode.globalTransform = parentNode.globalTransform;
            }

            pChildNode = &mNodes.Insert(entity, entityNode);
            parentNode.children.PushBack(pChildNode);
        }
        else
        {
            pChildNode = &mNodes.Get(entity);
            SceneNode* pChildParentNode = pChildNode->pParent;

            // TODO: Find a way to speed this up?
            // If node exists, Find() should never fail
            Array<SceneNode*>::Iterator it = pChildParentNode->children.Find(pChildNode);
            pChildParentNode->children.Remove(it);

            parentNode.children.PushBack(pChildNode);

            UpdateBranch(pChildNode);
        }

        return true;
    }

    bool EntityGraph::ParentEntityToRoot(Entity entity)
    {
        return ParentEntity(entity, NullEntity);
    }

    void EntityGraph::Update(Entity entity)
    {
        if (mNodes.Contains(entity))
        {
            UpdateBranch(&mNodes.Get(entity));
        }
    }

    EntityGraphView EntityGraph::CreateView(Entity entity)
    {
        if (mNodes.Contains(entity))
        {
            SceneNode* pNode = &mNodes.Get(entity);
            return EntityGraphView(this, pNode);
        }

        // TODO: find a better fail state?
        return CreateRootView();
    }

    EntityGraphView EntityGraph::CreateRootView()
    {
        return EntityGraphView(this, mpRoot);
    }




    //// TEMP

    TransformComponentOld::TransformComponentOld() :
        position({ 0.0f, 0.0f, 0.0f }),
        rotation(Quatf().SetEuler({ 0.0f, 0.0f, 0.0f })),
        scale({ 1.0f, 1.0f, 1.0f })
    {
        // Nothing
    }

    TransformComponentOld::TransformComponentOld(const Vec3f& position, const Quatf& rotation, const Vec3f& scale) :
        position(position),
        rotation(rotation),
        scale(scale)
    {
        // Nothing
    }

    Vec3f TransformComponentOld::GetForward()
    {
        return rotation * Vec3f(0.0f, 0.0f, 1.0f);
    }

    Vec3f TransformComponentOld::GetBackward()
    {
        return rotation * Vec3f(0.0f, 0.0f, -1.0f);
    }

    Vec3f TransformComponentOld::GetLeft()
    {
        return rotation * Vec3f(1.0f, 0.0f, 0.0f);
    }

    Vec3f TransformComponentOld::GetRight()
    {
        return rotation * Vec3f(-1.0f, 0.0f, 1.0f);
    }

    Vec3f TransformComponentOld::GetUp()
    {
        return rotation * Vec3f(0.0f, 1.0f, 1.0f);
    }

    Vec3f TransformComponentOld::GetDown()
    {
        return rotation * Vec3f(0.0f, -1.0f, 1.0f);
    }

    Mat4f TransformComponentOld::GetMatrix()
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
}

