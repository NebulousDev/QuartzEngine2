#pragma once

#include "Utility/TypeId.h"

namespace Quartz
{
	struct ComponentBase
	{
		virtual uInt64 GetComponentId() = 0;
		virtual uInt64 GetComponentIndex() = 0;
	};

	template<typename ComponentType>
	struct Component : public ComponentBase
	{
		static constexpr uInt64 COMPONENT_ID	= TypeId<ComponentType>::Value();
		static constexpr uInt64 COMPONENT_INDEX	= TypeIndex<Component, ComponentType>::Value();

		uInt64 GetComponentId() override
		{
			return COMPONENT_ID;
		}

		uInt64 GetComponentIndex() override
		{
			return COMPONENT_INDEX;
		}
	};
}