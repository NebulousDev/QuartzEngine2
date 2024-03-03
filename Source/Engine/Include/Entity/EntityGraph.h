#pragma once

#include "EngineAPI.h"

#include "Entity.h"
#include "Types/Special/BlockSet.h"
#include "Types/Array.h"

#include "Math/Math.h"
#include "Component.h"

namespace Quartz
{
	class EntityGraph;
	class EntityDatabase;

	// TEMP
	struct QUARTZ_ENGINE_API TransformComponentOld : public Component<TransformComponentOld>
	{
		Vec3f position;
		Quatf rotation;
		Vec3f scale;

		TransformComponentOld();
		TransformComponentOld(const Vec3f& position, const Quatf& rotation, const Vec3f& scale);

		Vec3f GetForward();
		Vec3f GetBackward();
		Vec3f GetLeft();
		Vec3f GetRight();
		Vec3f GetUp();
		Vec3f GetDown();

		Mat4f GetMatrix();
	};

	struct SceneNode
	{
		SceneNode*			pParent;
		Entity				entity;
		TransformComponentOld* pLocalTransform;
		Mat4f				globalTransform;
		Array<SceneNode*>	children;
	};

	class QUARTZ_ENGINE_API EntityGraphView
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

	class QUARTZ_ENGINE_API EntityGraph
	{
	private:
		friend class EntityWorld;

		using NodeStorage = BlockSet<SceneNode, Entity, Entity::HandleIntType, 1024>;

	private:
		EntityDatabase*		mpDatabase;
		NodeStorage			mNodes;
		SceneNode*			mpRoot;
		TransformComponentOld 	mZeroTransform;

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