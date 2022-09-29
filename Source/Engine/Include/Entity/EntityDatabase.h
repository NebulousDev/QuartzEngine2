#pragma once

#include "Quartz.h"

#include "Types/Array.h"
#include "Types/Map.h"
#include "Types/BlockSet.h"
#include "Utility/TypeId.h"
#include "Utility/Move.h"

#include "Entity.h"
#include "EntityView.h"

namespace Quartz
{
	template<>
	inline Entity::HandleIntType SparseIndex(Entity& entity)
	{
		return entity.index; // - 1 ?
	}

	class QUARTZ_API EntityDatabase
	{
	public:

		template<typename ComponentType>
		using ComponentStorage = BlockSet<ComponentType, Entity, Entity::HandleIntType>;
		using EntitySet	       = SparseSet<Entity, Entity::HandleIntType>;

	private:
		static uSize GetLinearTypeIndex(const String& componentName);

		template<typename ComponentType>
		struct ComponentTypeIndex
		{
			static uSize Value()
			{
				static uSize index = GetLinearTypeIndex(TypeName<ComponentType>::Value());
				return index;
			}
		};

	private:
		Array<EntitySet*>	mStorageSets;
		Array<Entity>		mEntites;

	private:
		template<typename Component>
		bool HasComponentImpl(Entity entity)
		{
			using ComponentType = std::decay_t<Component>;
			uSize typeIndex = ComponentTypeIndex<ComponentType>::Value();

			if (typeIndex >= mStorageSets.Size())
			{
				return false;
			}

			ComponentStorage<Component>& storage = 
				*static_cast<ComponentStorage<Component>*>(mStorageSets[typeIndex]);

			return storage.Contains(entity);
		}

		template<typename Component>
		void AddComponentImpl(Entity entity, Component&& component)
		{
			using ComponentType = std::decay_t<Component>;
			uSize typeIndex = ComponentTypeIndex<ComponentType>::Value();

			if (typeIndex >= mStorageSets.Size())
			{
				mStorageSets.Resize(typeIndex + 1, nullptr);
				mStorageSets[typeIndex] = new ComponentStorage<ComponentType>();
			}

			static_cast<ComponentStorage<ComponentType>*>
				(mStorageSets[typeIndex])->Insert(entity, Forward<Component>(component));
		}

		template<typename Component>
		void RemoveComponentImpl(Entity entity)
		{
			using ComponentType = std::decay_t<Component>;
			uSize typeIndex = ComponentTypeIndex<ComponentType>::Value();
			static_cast<ComponentStorage<ComponentType>*>(mStorageSets[typeIndex])->Remove(entity);
		}

	public:

		// NOTE/TODO: Does not check for NullEntity!
		bool EntityExists(Entity entity)
		{
			if ((entity.index - 1) > mEntites.Size())
			{
				return false;
			}
			
			return mEntites[entity.index - 1].version == entity.version;
		}

		template<typename... Component>
		Entity CreateEntity(Component&&... component)
		{
			Entity entity = mEntites.PushBack(Entity(mEntites.Size() + 1, 0));
			AddComponent(entity, Forward<Component>(component)...);
			return entity;
		}

		template<typename... Component>
		void AddComponent(Entity entity, Component&&... component)
		{
			(AddComponentImpl<Component>(entity, Forward<Component>(component)), ...);
		}

		template<typename... Component>
		void RemoveComponent(Entity entity)
		{
			(RemoveComponentImpl<Component>(entity), ...);
		}

		template<typename... Component>
		bool HasComponent(Entity entity)
		{
			return (HasComponentImpl<Component>(entity) && ...);
		}

		/* Assumes entity has component. Undefiened otherwise.*/
		template<typename Component>
		Component& GetComponent(Entity entity)
		{
			using ComponentType = std::decay_t<Component>;
			uSize typeIndex = ComponentTypeIndex<ComponentType>::Value();
			return static_cast<ComponentStorage<ComponentType>*>(mStorageSets[typeIndex])->Get(entity);
		}

		template<typename Component>
		bool ComponentExists()
		{
			return ComponentTypeIndex<Component>::Value() < mStorageSets.Size();
		}

		template<typename... Component>
		EntityView<Component...> CreateView()
		{
			if ((!ComponentExists<Component>() || ...))
			{
				// One or more components does not exist in this world
				return EntityView<Component...>();
			}

			return EntityView<Component...>(
				static_cast<ComponentStorage<Component>*>(mStorageSets[ComponentTypeIndex<Component>::Value()])...);
		}
	};
}