#include "Entity/EntityDatabase.h"
#include "Types/Map.h"
#include "Types/String.h"

namespace Quartz
{
	Map<String, uSize> linearIndexMap;
	uSize currentCount = 0;

	uSize EntityDatabase::GetLinearTypeIndex(const String& componentName)
	{
		auto& itr = linearIndexMap.Find(componentName);
		if (itr != linearIndexMap.End())
		{
			return itr->value;
		}

		return linearIndexMap.Put(componentName, currentCount++);
	}
}
