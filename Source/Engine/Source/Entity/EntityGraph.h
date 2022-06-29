#pragma once

#include "Entity.h"
#include "Component/Transform.h"
#include "Types/BlockSet.h"
#include "Types/Array.h"

namespace Quartz
{
	class EntityGraph;
	class EntityDatabase;

	struct SceneNode
	{
		SceneNode*			pParent;
		Entity				entity;
		TransformComponent* pLocalTransform;
		Mat4f				globalTransform;
		Array<SceneNode*>	children;
	};

	class EntityGraphView
	{
	private:
		EntityGraph* mpParentGraph;
		SceneNode*	mpViewRoot;

	public:
		EntityGraphView(EntityGraph* pParentGraph, SceneNode* pRoot);

		SceneNode* GetNode(Entity entity);

		inline SceneNode* GetRoot() { return mpViewRoot; }
	};

	// TODO: transition away from linked lists to a more compact
	//		 data structure (indexed array?)

	class EntityGraph
	{
	private:
		friend class EntityWorld;

		using NodeStorage = BlockSet<SceneNode, Entity, Entity::HandleIntType, 1024>;

	private:
		EntityDatabase*		mpDatabase;
		NodeStorage			mNodes;
		SceneNode*			mpRoot;
		TransformComponent	mZeroTransform;

	private:
		EntityGraph();

		void UpdateBranch(SceneNode* node);
	
	public:
		EntityGraph(EntityDatabase* pDatabase);

		void SetDatabase(EntityDatabase* pDatabase);

		SceneNode* GetNode(Entity entity);

		bool ParentEntity(Entity entity, Entity parent);
		bool ParentEntityToRoot(Entity entity);

		void Update(Entity entity);

		EntityGraphView CreateView(Entity entity);
		EntityGraphView CreateRootView();
	};
}