#include "Runtime/Runtime.h"

namespace Quartz
{
	void Runtime::RegisterUpdate(RuntimeUpdateFunc updateFunc)
	{
		updates.PushBack(updateFunc);
	}

	void Runtime::UpdateAll()
	{
		for (RuntimeUpdateFunc& updateFunc : updates)
		{
			updateFunc(0.0);
		}
	}
}

