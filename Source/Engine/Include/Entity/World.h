#pragma once

#include "EngineAPI.h"
#include "EntityDatabase.h"
#include "EntityGraph.h"
#include "Runtime/Runtime.h"

#include "Log.h"

namespace Quartz
{
	class EntityWorld;

	struct EntityCreatedEvent
	{
		EntityWorld& world;
		Entity entity;
	};

	struct EntityDestroyedEvent
	{
		EntityWorld& world;
		Entity entity;
	};

	template<typename Component>
	struct ComponentAddedEvent
	{
		EntityWorld& world;
		Entity entity;
		Component& component;
	};

	template<typename Component>
	struct ComponentRemovedEvent
	{
		EntityWorld& world;
		Entity entity;
		Component component;
	};

	class QUARTZ_ENGINE_API EntityWorld
	{
		friend class Engine;

	private:
		EntityDatabase*	mpDatabase;
		EntityGraph*	mpGraph;
		Entity			mSingleton;

	private:
		void Initialize(EntityDatabase* pDatabase, EntityGraph* pGraph);

		Runtime& GetEngineRuntime();

		void TriggerEntityCreatedEvent(Entity entity)
		{
			EntityCreatedEvent event{ *this, entity };
			GetEngineRuntime().Trigger<EntityCreatedEvent>(event);
		}

		void TriggerEntityDestroyedEvent(Entity entity)
		{
			EntityDestroyedEvent event{ *this, entity };
			GetEngineRuntime().Trigger<EntityDestroyedEvent>(event);
		}

		template<typename Component>
		void TriggerComponentAddedEvent(Entity entity, Component&& component)
		{
			ComponentAddedEvent<Component> event{ *this, entity, component };
			GetEngineRuntime().Trigger<ComponentAddedEvent<Component>>(event);
		}

		template<typename Component>
		void TriggerComponentRemovedEvent(Entity entity, Component&& component)
		{
			ComponentRemovedEvent<Component> event{ *this, entity, component };
			GetEngineRuntime().Trigger<ComponentRemovedEvent<Component>>(event);
		}

		template<typename Component>
		Component& AddComponentImpl(Entity entity, Component&& component)
		{
			Component& newComponent = mpDatabase->AddComponent(entity, Forward<Component>(component));
			TriggerComponentAddedEvent<Component>(entity, component);
			return newComponent;
		}

		template<typename Component>
		void RemoveComponentImpl(Entity entity)
		{
			mpDatabase->RemoveComponent<Component>(entity);
		}

		template<typename Component>
		bool HasComponentImpl(Entity entity)
		{
			return mpDatabase->HasComponent<Component>(entity);
		}

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
			Entity entity = mpDatabase->CreateEntity();
			//mpGraph->ParentEntityToRoot(entity);

			(AddComponentImpl<Component>(entity, Forward<Component>(component)), ...);

			TriggerEntityCreatedEvent(entity);

			return entity;
		}

		// @TODO: DestroyEntity

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
			// @TODO: check?
			return mpDatabase->GetComponent<Component>(mSingleton);
		}

		/* Assumes entity has component. Undefiened otherwise.*/
		template<typename Component>
		Component& Get(Entity entity)
		{
			if (!IsValid(entity))
			{
				LogWarning("Attempted to get component from invalid entity [%X]", entity);
			}

			return mpDatabase->GetComponent<Component>(entity);
		}

		template<typename Component>
		Component& AddComponent(Entity entity, Component&& component)
		{
			if (!IsValid(entity))
			{
				LogWarning("Attempted to add component to invalid entity [%X]", entity);
			}

			return AddComponentImpl(entity, Forward<Component>(component));
		}

		template<typename... Component>
		void AddComponents(Entity entity, Component&&... component)
		{
			if (!IsValid(entity))
			{
				LogWarning("Attempted to add component to invalid entity [%X]", entity);
				return;
			}
			
			(AddComponentImpl<Component>(entity, Forward<Component>(component)), ...);
		}

		template<typename Component>
		void RemoveComponent(Entity entity)
		{
			if (!IsValid(entity))
			{
				LogWarning("Attempted to remove component from invalid entity [%X]", entity);
				return;
			}

			RemoveComponentImpl<Component>(entity);
		}

		template<typename... Component>
		void RemoveComponents(Entity entity)
		{
			if (!IsValid(entity))
			{
				LogWarning("Attempted to remove component from invalid entity [%X]", entity);
				return;
			}

			(RemoveComponentImpl<Component>(entity), ...);
		}

		bool HasEntity(Entity entity);

		template<typename Component>
		bool HasComponent(Entity entity)
		{
			if (!IsValid(entity))
			{
				LogWarning("Attempted to find component in invalid entity [%X]", entity);
				return false;
			}

			return HasComponentImpl<Component>(entity);
		}

		template<typename... Component>
		bool HasComponents(Entity entity)
		{
			if (!IsValid(entity))
			{
				LogWarning("Attempted to find component in invalid entity [%X]", entity);
				return false;
			}

			return (HasComponentImpl<Component>(entity) && ...);
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