#include "Entity/EntityDatabase.h"
#include "Types/Map.h"
#include "Types/String.h"

namespace Quartz
{
	uSize EntityDatabase::GetTypeIndex(const String& componentName)
	{
		auto& itr = mTypeIndexMap.Find(componentName);
		if (itr != mTypeIndexMap.End())
		{
			return itr->value;
		}

		return mTypeIndexMap.Put(componentName, mTypeCount++);
	}

	EntityDatabase::EntityDatabase()
	{
		mStorageSets.Reserve(64);
		mEntites.Reserve(1024);
	};
}
