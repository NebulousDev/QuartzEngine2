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

		SystemQueryFunc		queryFunc		= (SystemQueryFunc)pLibrary->GetFunction("SystemQuery", false);
		SystemLoadFunc		loadFunc		= (SystemLoadFunc)pLibrary->GetFunction("SystemLoad", true);
		SystemUnloadFunc	unloadFunc		= (SystemUnloadFunc)pLibrary->GetFunction("SystemUnload", true);
		SystemPreInitFunc	preInitFunc		= (SystemPreInitFunc)pLibrary->GetFunction("SystemPreInit", true);
		SystemInitFunc		initFunc		= (SystemInitFunc)pLibrary->GetFunction("SystemInit", true);
		SystemPostInitFunc	postInitFunc	= (SystemPostInitFunc)pLibrary->GetFunction("SystemPostInit", true);
		SystemShutdownFunc	shutdownFunc	= (SystemShutdownFunc)pLibrary->GetFunction("SystemShutdown", true);

		/* Required Query Function */
		if (!queryFunc)
		{
			LogError("Failed to create system from dynamic library [path='%s']: "
				"Required 'SystemQuery' function was not found.", pLibrary->GetPath().Str());

			return nullptr;
		}

		/* Optional Load Function */
		if (!loadFunc)
		{
			LogWarning("Warning while creating system from dynamic library [path='%s']: "
				"Optional 'SystemLoad' function was not found.", pLibrary->GetPath().Str());
		}

		/* Optional Unload Function */
		if (!unloadFunc)
		{
			LogWarning("Warning while creating system from dynamic library [path='%s']: "
				"Optional 'SystemUnload' function was not found.", pLibrary->GetPath().Str());
		}

		/* Optional PreInit Function */
		if (!preInitFunc)
		{
			LogWarning("Warning while creating system from dynamic library [path='%s']: "
				"Optional 'SystemPreInit' function was not found.", pLibrary->GetPath().Str());
		}

		/* Optional Init Function */
		if (!initFunc)
		{
			LogWarning("Warning while creating system from dynamic library [path='%s']: "
				"Optional 'SystemInit' function was not found.", pLibrary->GetPath().Str());
		}

		/* Optional PostInit Function */
		if (!postInitFunc)
		{
			LogWarning("Warning while creating system from dynamic library [path='%s']: "
				"Optional 'SystemPost' function was not found.", pLibrary->GetPath().Str());
		}

		/* Optional Shutdown Function */
		if (!shutdownFunc)
		{
			LogWarning("Warning while creating system from dynamic library [path='%s']: "
				"Optional 'SystemShutdown' function was not found.", pLibrary->GetPath().Str());
		}

		// Query system for name data etc

		SystemQueryInfo queryInfo = {};
		queryInfo.name = "Uninitialized System";
		queryInfo.version = "0.0.0";

		bool queryResult = queryFunc(false, queryInfo);

		LogInfo("Querying system [path='%s']: %s", pLibrary->GetPath().Str(), queryResult ? "true" : "false");

		return new System(
			pLibrary, 
			queryResult, 
			queryInfo, 
			queryFunc, 
			loadFunc, 
			unloadFunc,
			preInitFunc,
			initFunc,
			postInitFunc,
			shutdownFunc);
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
			LogWarning("Attempted to load an already loaded system "
				"[name=%s, version=%s]", pSystem->GetName().Str(), pSystem->GetVersion().Str());
			return true;
		}

		if (!pSystem->Load(Log::GetGlobalLog()))
		{
			LogError("Failed to load system: ['%s', version='%s']: "
				"SystemLoad() returned false. Skipping...", pSystem->GetName().Str(), pSystem->GetVersion().Str());
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

	bool SystemAdmin::PreInitSystem(System* pSystem)
	{
		pSystem->PreInit();

		return true;
	}

	bool SystemAdmin::InitSystem(System* pSystem)
	{
		pSystem->Init();

		return true;
	}

	bool SystemAdmin::PostInitSystem(System* pSystem)
	{
		pSystem->PostInit();

		return true;
	}

	bool SystemAdmin::ShutdownSystem(System* pSystem)
	{
		pSystem->Shutdown();

		return true;
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

	void SystemAdmin::PreInitAll()
	{
		for (System* pSystem : smSystemRegistry)
		{
			PreInitSystem(pSystem);
		}
	}

	void SystemAdmin::InitAll()
	{
		for (System* pSystem : smSystemRegistry)
		{
			InitSystem(pSystem);
		}
	}

	void SystemAdmin::PostInitAll()
	{
		for (System* pSystem : smSystemRegistry)
		{
			PostInitSystem(pSystem);
		}
	}

	void SystemAdmin::ShutdownAll()
	{
		for (System* pSystem : smSystemRegistry)
		{
			ShutdownSystem(pSystem);
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