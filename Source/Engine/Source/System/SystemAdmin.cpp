#include "SystemAdmin.h"

namespace Quartz
{
	System* SystemAdmin::LoadSystem(const char* dllPath)
	{
		return nullptr;
	}

	void SystemAdmin::UnloadSystem(System* pSystem)
	{

	}

	bool SystemAdmin::QuerySystem(const System* pSystem)
	{
		return pSystem->query(false);
	}
}