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

        mZeroTransform = TransformComponent(
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

        SceneNode& parentNode = mNodes.Get(parent);
        SceneNode* pChildNode = nullptr;

        if (!mNodes.Contains(entity))
        {
            SceneNode entityNode;
            entityNode.pParent = &parentNode;
            entityNode.entity = entity;

            if (mpDatabase->HasComponent<TransformComponent>(entity))
            {
                TransformComponent& transform = mpDatabase->GetComponent<TransformComponent>(entity);
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
}

