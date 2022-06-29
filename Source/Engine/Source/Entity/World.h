#pragma once

#include "EntityDatabase.h"
#include "EntityGraph.h"

#include "Log.h"

namespace Quartz
{
	class EntityWorld
	{
		friend class Engine;

	private:
		EntityDatabase*	mpDatabase;
		EntityGraph*	mpGraph;

	private:
		void Initialize(EntityDatabase* pDatabase, EntityGraph* pGraph);

	public:
		EntityWorld();
		EntityWorld(EntityDatabase* pDatabase, EntityGraph* pGraph);

		bool IsValid(Entity entity);
		bool SetParent(Entity entity, Entity parent);
		bool RemoveParent(Entity entity);

		void Refresh(Entity entity);

		template<typename... Component>
		Entity CreateEntity(Component&&... component)
		{
			Entity entity = mpDatabase->CreateEntity(Forward<Component>(component)...);
			mpGraph->ParentEntityToRoot(entity);
			return entity;
		}

		template<typename... Component>
		Entity CreateEntityParented(Entity parent, Component&&... component)
		{
			Entity entity = mpDatabase->CreateEntity(Forward<Component>(component)...);

			if (!mpGraph->ParentEntity(entity, parent))
			{
				mpGraph->ParentEntityToRoot(entity);
				LogWarning("Attempted to parent entity [%X] to invalid parent [%X]!", entity, parent);
			}

			return entity;
		}

		template<typename... Component>
		void AddComponent(Entity entity, Component&&... component)
		{
			if (!IsValid(entity))
			{
				LogWarning("Attempted to add component to invalid entity [%X]", entity);
				return;
			}
			
			mpDatabase->AddComponent(entity, Forward<Component>(component)...);
		}

		template<typename... Component>
		void RemoveComponent(Entity entity)
		{
			mpDatabase->RemoveComponent<Component...>(entity);
		}

		bool HasEntity(Entity entity);

		template<typename... Component>
		bool HasComponent(Entity entity)
		{
			return mpDatabase->HasComponent<Component...>(entity);
		}

		/* Assumes entity has component. Undefiened otherwise.*/
		template<typename Component>
		Component& GetComponent(Entity entity)
		{
			return mpDatabase->GetComponent<Component>(entity);
		}

		template<typename... Component>
		EntityView<Component...> CreateView()
		{
			return mpDatabase->CreateView<Component...>();
		}

		EntityDatabase& GetDatabase();
		EntityGraph& GetGraph();
	};
}