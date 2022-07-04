#include "SystemAdmin.h"

#include "Log.h"
#include <cassert>

namespace Quartz
{
	Array<System*> SystemAdmin::smSystemRegistry;

	System* SystemAdmin::CreateSystem(DynamicLibrary* pLibrary)
	{
		if (!pLibrary)
		{
			LogError("Failed to create system: CreateSystem() was passed a null library.");
			return nullptr;
		}

		SystemQueryFunc		queryFunc	= (SystemQueryFunc)pLibrary->GetFunction("SystemQuery");
		SystemLoadFunc		loadFunc	= (SystemLoadFunc)pLibrary->GetFunction("SystemLoad");
		SystemUnloadFunc	unloadFunc	= (SystemUnloadFunc)pLibrary->GetFunction("SystemUnload");

		if (!queryFunc)
		{
			LogError("Failed to create system from dynamic library [path='%s']: \
				Missing 'SystemQuery' function.", pLibrary->GetPath().Str());

			return nullptr;
		}

		if (!loadFunc)
		{
			LogError("Failed to create system from dynamic library [path='%s']: \
				Missing 'SystemLoad' function.", pLibrary->GetPath().Str());

			return nullptr;
		}

		if (!unloadFunc)
		{
			LogError("Failed to create system from dynamic library [path='%s']: \
				Missing 'SystemUnload' function.", pLibrary->GetPath().Str());

			return nullptr;
		}

		// Query system for name data etc

		SystemQueryInfo queryInfo = {};
		queryInfo.name = "Uninitialized System";
		queryInfo.version = "0.0.0";

		bool queryResult = queryFunc(false, queryInfo);

		LogInfo("Querying system [path='%s']: %s", pLibrary->GetPath().Str(), queryResult ? "true" : "false");		

		return new System(pLibrary, queryResult, queryInfo, queryFunc, loadFunc, unloadFunc);
	}

	void SystemAdmin::DestroySystem(System* pSystem)
	{
		UnregisterSystem(pSystem);
		delete pSystem;
	}

	System* SystemAdmin::CreateAndRegisterSystem(DynamicLibrary* pLibrary)
	{
		System* pSystem = CreateSystem(pLibrary);

		if (pSystem)
		{
			RegisterSystem(pSystem);
		}

		return pSystem;
	}

	void SystemAdmin::RegisterSystem(System* pSystem)
	{
		assert(pSystem && "RegisterSystem() was provided a null system.");

		smSystemRegistry.PushBack(pSystem);

		LogTrace("Registered system: ['%s', version='%s']", pSystem->GetName().Str(), pSystem->GetVersion().Str());
	}

	void SystemAdmin::UnregisterSystem(System* pSystem)
	{
		assert(pSystem && "UnregisterSystem() was provided a null system.");

		pSystem->Unload();

		auto it = smSystemRegistry.Find(pSystem);
		if (it != smSystemRegistry.End())
		{
			smSystemRegistry.Remove(it);
		}

		delete pSystem;
	}

	bool SystemAdmin::LoadSystem(System* pSystem)
	{
		assert(pSystem && "LoadSystem() was provided a null system.");

		if (pSystem->IsLoaded())
		{
			LogWarning("Attempted to load an already loaded system\
				[name=%s, version=%s]", pSystem->GetName().Str(), pSystem->GetVersion().Str());
			return true;
		}

		if (!pSystem->Load())
		{
			LogError("Failed to load system: ['%s', version='%s']: \
				SystemLoad() returned false. Skipping...", pSystem->GetName().Str(), pSystem->GetVersion().Str());
			return false;
		}

		pSystem->SetLoaded(true);

		LogInfo("Loaded system: ['%s', version='%s']", pSystem->GetName().Str(), pSystem->GetVersion().Str());

		return true;
	}

	void SystemAdmin::UnloadSystem(System* pSystem)
	{
		assert(pSystem && "UnloadSystem() was provided a null system.");

		pSystem->Unload();
		pSystem->SetLoaded(false);

		LogInfo("Unloaded system: ['%s', version='%s']", pSystem->GetName().Str(), pSystem->GetVersion().Str());
	}

	void SystemAdmin::LoadAll()
	{
		for (System* pSystem : smSystemRegistry)
		{
			LoadSystem(pSystem);
		}
	}

	void SystemAdmin::UnloadAll()
	{
		for (System* pSystem : smSystemRegistry)
		{
			UnloadSystem(pSystem);
		}
	}

	void SystemAdmin::DestroyAll()
	{
		for (System* pSystem : smSystemRegistry)
		{
			delete pSystem;
		}

		smSystemRegistry.Clear();
	}
}