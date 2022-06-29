#include "World.h"

namespace Quartz
{
	EntityWorld::EntityWorld()
		: mpDatabase(nullptr),
		mpGraph(nullptr)
	{
		// Nothing
	}

	void EntityWorld::Initialize(EntityDatabase* pDatabase, EntityGraph* pGraph)
	{
		mpDatabase = pDatabase;
		mpGraph = pGraph;
	}

	EntityWorld::EntityWorld(EntityDatabase* pDatabase, EntityGraph* pGraph)
		: mpDatabase(pDatabase), mpGraph(pGraph)
	{
		// Nothing
	}

	bool EntityWorld::IsValid(Entity entity)
	{
		if (entity == NullEntity)
		{
			return false;
		}

		return mpDatabase->EntityExists(entity);
	}

	bool EntityWorld::SetParent(Entity entity, Entity parent)
	{
		if (!IsValid(entity))
		{
			LogWarning("Attempted to parent invalid entity [%X]", entity);
			return false;
		}

		if (!IsValid(parent))
		{
			LogWarning("Attempted to parent entity [%X] to invalid parent [%X]!", entity, parent);
			return false;
		}

		mpGraph->ParentEntity(entity, parent);

		return true;
	}

	bool EntityWorld::RemoveParent(Entity entity)
	{
		if (!IsValid(entity))
		{
			LogWarning("Attempted to parent invalid entity [%X]", entity);
			return false;
		}

		mpGraph->ParentEntityToRoot(entity);

		return true;
	}

	void EntityWorld::Refresh(Entity entity)
	{
		mpGraph->Update(entity);
	}

	bool EntityWorld::HasEntity(Entity entity)
	{
		return mpDatabase->EntityExists(entity);
	}

	EntityDatabase& EntityWorld::GetDatabase()
	{ 
		return *mpDatabase;
	}

	EntityGraph& EntityWorld::GetGraph()
	{
		return *mpGraph;
	}
}

