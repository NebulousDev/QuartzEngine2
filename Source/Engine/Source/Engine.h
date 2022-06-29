#pragma once

#include "Entity/World.h"

namespace Quartz
{
	class Engine
	{
	private:
		static EntityWorld smSystemWorld;
		static EntityWorld smGameWorld;

	public:
		//static EntityWorld& GetSystemWorld();
		//static EntityWorld& GetGameWorld();
	};
}