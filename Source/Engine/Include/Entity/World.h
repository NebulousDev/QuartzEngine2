#pragma once

#include "EngineAPI.h"

#include "EntityDatabase.h"
#include "EntityGraph.h"

#include "Log.h"

namespace Quartz
{
	class QUARTZ_ENGINE_API EntityWorld
	{
		friend class Engine;

	private:
		EntityDatabase*	mpDatabase;
		EntityGraph*	mpGraph;
		Entity			mSingleton;

	private:
		void Initialize(EntityDatabase* pDatabase, EntityGraph* pGraph);

#if 0
		/* Assumes singleton component exists. Undefiened otherwise.*/
		template<typename Component>
		Component& GetSingleton()
		{
			return mpDatabase->GetComponent<Component>(mSingleton);
		}

		/* Assumes entity has component. Undefiened otherwise.*/
		template<typename Component>
		Component& GetComponent(Entity entity)
		{
			return mpDatabase->GetComponent<Component>(entity);
		}
#endif

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
			//mpGraph->ParentEntityToRoot(entity);
			return entity;
		}

		template<typename... Component>
		Entity CreateEntityParented(Entity parent, Component&&... component)
		{
			Entity entity = mpDatabase->CreateEntity(Forward<Component>(component)...);

			if (!mpGraph->ParentEntity(entity, parent))
			{
				//mpGraph->ParentEntityToRoot(entity);
				LogWarning("Attempted to parent entity [%X] to invalid parent [%X]!", entity, parent);
			}

			return entity;
		}

		template<typename Component>
		Component& CreateSingleton(Component&& component)
		{
			if (!mpDatabase->HasComponent<Component>(mSingleton))
			{
				mpDatabase->AddComponent(mSingleton, Forward<Component>(component));
			}

			return mpDatabase->GetComponent<Component>(mSingleton);
		}

		template<typename Component>
		Component& CreateSingleton()
		{
			return CreateSingleton<Component>(Component{});
		}

		template<typename Component>
		void DestroySingleton()
		{
			mpDatabase->RemoveComponent<Component>(mSingleton);
		}

		/* Assumes singleton component exists. Undefiened otherwise.*/
		template<typename Component>
		Component& Get()
		{
			return mpDatabase->GetComponent<Component>(mSingleton);
		}

		/* Assumes entity has component. Undefiened otherwise.*/
		template<typename Component>
		Component& Get(Entity entity)
		{
			return mpDatabase->GetComponent<Component>(entity);
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

		template<typename... Component>
		EntityView<Component...> CreateView()
		{
			return mpDatabase->CreateView<Component...>();
		}

		EntityDatabase& GetDatabase();
		EntityGraph& GetGraph();

		inline const uSize EntityCount() const { return mpDatabase->EntityCount(); }
	};
}