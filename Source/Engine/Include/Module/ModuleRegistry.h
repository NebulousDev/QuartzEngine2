#pragma once

#include "Module.h"
#include "Types/Array.h"

namespace Quartz
{
	class Engine;

	class ModuleRegistry
	{
	private:
		Array<Module*> mModuleRegistry;

	public:
		Module* CreateModule(DynamicLibrary* pLibrary);
		void DestroyModule(Module* pModule);

		Module* CreateAndRegisterModule(DynamicLibrary* pLibrary);

		void RegisterModule(Module* pModule);
		void UnregisterModule(Module* pModule);

		bool LoadModule(Module* pModule, Log& engineLog, Engine& engine);
		void UnloadModule(Module* pModule);

		bool PreInitModule(Module* pModule);
		bool InitModule(Module* pModule);
		bool PostInitModule(Module* pModule);
		bool ShutdownModule(Module* pModule);

		void LoadAll(Log& engineLog, Engine& engine);
		void UnloadAll();

		void PreInitAll();
		void InitAll();
		void PostInitAll();
		void ShutdownAll();

		void DestroyAll();
	};
}