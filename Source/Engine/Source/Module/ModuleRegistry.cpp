#include "Module/ModuleRegistry.h"

#include "Log.h"
#include <cassert>

namespace Quartz
{
	Module* ModuleRegistry::CreateModule(DynamicLibrary* pLibrary)
	{
		if (!pLibrary)
		{
			LogError("Failed to create module: CreateModule() was passed a null library.");
			return nullptr;
		}

		ModuleQueryFunc		queryFunc		= (ModuleQueryFunc)pLibrary->GetFunction("ModuleQuery", false);
		ModuleLoadFunc		loadFunc		= (ModuleLoadFunc)pLibrary->GetFunction("ModuleLoad", true);
		ModuleUnloadFunc	unloadFunc		= (ModuleUnloadFunc)pLibrary->GetFunction("ModuleUnload", true);
		ModulePreInitFunc	preInitFunc		= (ModulePreInitFunc)pLibrary->GetFunction("ModulePreInit", true);
		ModuleInitFunc		initFunc		= (ModuleInitFunc)pLibrary->GetFunction("ModuleInit", true);
		ModulePostInitFunc	postInitFunc	= (ModulePostInitFunc)pLibrary->GetFunction("ModulePostInit", true);
		ModuleShutdownFunc	shutdownFunc	= (ModuleShutdownFunc)pLibrary->GetFunction("ModuleShutdown", true);

		/* Required Query Function */
		if (!queryFunc)
		{
			LogError("Failed to create module from dynamic library [path='%s']: "
				"Required 'ModuleQuery' function was not found.", pLibrary->GetPath().Str());

			return nullptr;
		}

		/* Optional Load Function */
		if (!loadFunc)
		{
			LogWarning("Warning while creating module from dynamic library [path='%s']: "
				"Optional 'ModuleLoad' function was not found.", pLibrary->GetPath().Str());
		}

		/* Optional Unload Function */
		if (!unloadFunc)
		{
			LogWarning("Warning while creating module from dynamic library [path='%s']: "
				"Optional 'ModuleUnload' function was not found.", pLibrary->GetPath().Str());
		}

		/* Optional PreInit Function */
		if (!preInitFunc)
		{
			LogWarning("Warning while creating module from dynamic library [path='%s']: "
				"Optional 'ModulePreInit' function was not found.", pLibrary->GetPath().Str());
		}

		/* Optional Init Function */
		if (!initFunc)
		{
			LogWarning("Warning while creating module from dynamic library [path='%s']: "
				"Optional 'ModuleInit' function was not found.", pLibrary->GetPath().Str());
		}

		/* Optional PostInit Function */
		if (!postInitFunc)
		{
			LogWarning("Warning while creating module from dynamic library [path='%s']: "
				"Optional 'ModulePostInit' function was not found.", pLibrary->GetPath().Str());
		}

		/* Optional Shutdown Function */
		if (!shutdownFunc)
		{
			LogWarning("Warning while creating module from dynamic library [path='%s']: "
				"Optional 'ModuleShutdown' function was not found.", pLibrary->GetPath().Str());
		}

		// Query module for name data etc

		ModuleQueryInfo queryInfo = {};
		queryInfo.name = "Uninitialized Module";
		queryInfo.version = "0.0.0";

		bool queryResult = queryFunc(false, queryInfo);

		LogInfo("Querying module [path='%s']: %s", pLibrary->GetPath().Str(), queryResult ? "true" : "false");

		return new Module(
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

	void ModuleRegistry::DestroyModule(Module* pModule)
	{
		UnregisterModule(pModule);
		delete pModule;
	}

	Module* ModuleRegistry::CreateAndRegisterModule(DynamicLibrary* pLibrary)
	{
		Module* pModule = CreateModule(pLibrary);

		if (pModule)
		{
			RegisterModule(pModule);
		}

		return pModule;
	}

	void ModuleRegistry::RegisterModule(Module* pModule)
	{
		assert(pModule && "RegisterModule() was provided a null module.");

		mModuleRegistry.PushBack(pModule);

		LogTrace("Registered module: ['%s', version='%s']", pModule->GetName().Str(), pModule->GetVersion().Str());
	}

	void ModuleRegistry::UnregisterModule(Module* pModule)
	{
		assert(pModule && "UnregisterModule() was provided a null module.");

		pModule->Unload();

		auto it = mModuleRegistry.Find(pModule);
		if (it != mModuleRegistry.End())
		{
			mModuleRegistry.Remove(it);
		}

		delete pModule;
	}

	bool ModuleRegistry::LoadModule(Module* pModule, Log& engineLog, Engine& engine)
	{
		assert(pModule && "LoadModule() was provided a null module.");

		if (pModule->IsLoaded())
		{
			LogWarning("Attempted to load an already loaded module "
				"[name=%s, version=%s]", pModule->GetName().Str(), pModule->GetVersion().Str());
			return true;
		}

		if (!pModule->Load(engineLog, engine))
		{
			LogError("Failed to load module: ['%s', version='%s']: "
				"ModuleLoad() returned false. Skipping...", pModule->GetName().Str(), pModule->GetVersion().Str());
			return false;
		}

		pModule->SetLoaded(true);

		LogInfo("Loaded module: ['%s', version='%s']", pModule->GetName().Str(), pModule->GetVersion().Str());

		return true;
	}

	void ModuleRegistry::UnloadModule(Module* pModule)
	{
		assert(pModule && "UnloadModule() was provided a null module.");

		pModule->Unload();
		pModule->SetLoaded(false);

		LogInfo("Unloaded module: ['%s', version='%s']", pModule->GetName().Str(), pModule->GetVersion().Str());
	}

	bool ModuleRegistry::PreInitModule(Module* pModule)
	{
		pModule->PreInit();

		return true;
	}

	bool ModuleRegistry::InitModule(Module* pModule)
	{
		pModule->Init();

		return true;
	}

	bool ModuleRegistry::PostInitModule(Module* pModule)
	{
		pModule->PostInit();

		return true;
	}

	bool ModuleRegistry::ShutdownModule(Module* pModule)
	{
		pModule->Shutdown();

		return true;
	}

	void ModuleRegistry::LoadAll(Log& engineLog, Engine& engine)
	{
		for (Module* pModule : mModuleRegistry)
		{
			LoadModule(pModule, engineLog, engine);
		}
	}

	void ModuleRegistry::UnloadAll()
	{
		for (Module* pModule : mModuleRegistry)
		{
			UnloadModule(pModule);
		}
	}

	void ModuleRegistry::PreInitAll()
	{
		for (Module* pModule : mModuleRegistry)
		{
			PreInitModule(pModule);
		}
	}

	void ModuleRegistry::InitAll()
	{
		for (Module* pModule : mModuleRegistry)
		{
			InitModule(pModule);
		}
	}

	void ModuleRegistry::PostInitAll()
	{
		for (Module* pModule : mModuleRegistry)
		{
			PostInitModule(pModule);
		}
	}

	void ModuleRegistry::ShutdownAll()
	{
		for (Module* pModule : mModuleRegistry)
		{
			ShutdownModule(pModule);
		}
	}

	void ModuleRegistry::DestroyAll()
	{
		for (Module* pModule : mModuleRegistry)
		{
			delete pModule;
		}

		mModuleRegistry.Clear();
	}
}