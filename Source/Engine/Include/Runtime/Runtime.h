#pragma once

#include "Quartz.h"
#include "Types/Array.h"

namespace Quartz
{
	typedef void (*RuntimeUpdateFunc)(double delta);

	class QUARTZ_API Runtime
	{
	private:
		Array<RuntimeUpdateFunc> updates;

	public:
		void RegisterUpdate(RuntimeUpdateFunc updateFunc);
		//void RegisterTick();
		//void RegisterTrigger();

		void UpdateAll();
	};
}
