#pragma once

#include "EngineAPI.h"

#include "Types/Array.h"
#include "Types/Map.h"
#include "Types/Special/BlockSet.h"
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

	class QUARTZ_ENGINE_API EntityDatabase
	{
	public:

		template<typename ComponentType>
		using ComponentStorage = BlockSet<ComponentType, Entity, Entity::HandleIntType>;
		using EntitySet	       = SparseSet<Entity, Entity::HandleIntType>;

	private:

		Map<String, uSize>	mTypeIndexMap;
		uSize				mTypeCount = 0;

		uSize GetTypeIndex(const String& componentName);

		template<typename Component>
		uSize GetTypeIndex()
		{
			using ComponentType = std::decay_t<Component>;
			static uSize index = GetTypeIndex(TypeName<ComponentType>::Value());
			return index;
		}

	private:
		Array<EntitySet*>	mStorageSets;
		Array<Entity>		mEntites;

	public:
		EntityDatabase();

		// @TODO: Does not check for NullEntity!
		bool EntityExists(Entity entity)
		{
			if ((entity.index - 1) > mEntites.Size())
			{
				return false;
			}
			
			return mEntites[entity.index - 1].version == entity.version;
		}

		inline Entity CreateEntity()
		{
			return mEntites.PushBack(Entity(mEntites.Size() + 1, 0));
		}

		template<typename Component>
		Component& AddComponent(Entity entity, Component&& component)
		{
			using ComponentType = std::decay_t<Component>;
			uSize typeIndex = GetTypeIndex<ComponentType>();

			if (typeIndex >= mStorageSets.Size())
			{
				mStorageSets.Resize(typeIndex + 1, nullptr);
				mStorageSets[typeIndex] = new ComponentStorage<ComponentType>();
			}

			ComponentStorage<ComponentType>* pStorage =
				static_cast<ComponentStorage<ComponentType>*>(mStorageSets[typeIndex]);

			return pStorage->Insert(entity, Forward<Component>(component));
		}

		template<typename Component>
		void RemoveComponent(Entity entity)
		{
			using ComponentType = std::decay_t<Component>;
			uSize typeIndex = GetTypeIndex<ComponentType>();
			static_cast<ComponentStorage<ComponentType>*>(mStorageSets[typeIndex])->Remove(entity);
		}

		template<typename Component>
		bool HasComponent(Entity entity)
		{
			using ComponentType = std::decay_t<Component>;
			uSize typeIndex = GetTypeIndex<ComponentType>();

			if (typeIndex >= mStorageSets.Size())
			{
				return false;
			}

			ComponentStorage<ComponentType>* pStorage =
				static_cast<ComponentStorage<ComponentType>*>(mStorageSets[typeIndex]);

			if (!pStorage)
			{
				return false;
			}

			return pStorage->Contains(entity);
		}

		// @TODO Assumes entity has component. Undefiened otherwise
		template<typename Component>
		Component& GetComponent(Entity entity)
		{
			using ComponentType = std::decay_t<Component>;
			uSize typeIndex = GetTypeIndex<ComponentType>();
			return static_cast<ComponentStorage<ComponentType>*>(mStorageSets[typeIndex])->Get(entity);
		}

		template<typename Component>
		bool ComponentExists()
		{
			using ComponentType = std::decay_t<Component>;
			return GetTypeIndex<ComponentType>() < mStorageSets.Size();
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
				static_cast<ComponentStorage<Component>*>(mStorageSets[GetTypeIndex<Component>()])...);
		}

		inline const uSize EntityCount() const { return mEntites.Size(); }
	};
}