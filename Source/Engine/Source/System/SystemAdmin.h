#pragma once

#include "System/System.h"
#include "Types/Array.h"

#include "System/DynamicLibrary.h"

namespace Quartz
{
	class SystemAdmin
	{
	private:
		static Array<System*> smSystemRegistry;

	public:
		static System* CreateSystem(DynamicLibrary* pLibrary);
		static void DestroySystem(System* pSystem);

		static System* CreateAndRegisterSystem(DynamicLibrary* pLibrary);

		static void RegisterSystem(System* pSystem);
		static void UnregisterSystem(System* pSystem);

		static bool LoadSystem(System* pSystem);
		static void UnloadSystem(System* pSystem);

		static bool PreInitSystem(System* pSystem);
		static bool InitSystem(System* pSystem);
		static bool PostInitSystem(System* pSystem);
		static bool ShutdownSystem(System* pSystem);

		static void LoadAll();
		static void UnloadAll();

		static void PreInitAll();
		static void InitAll();
		static void PostInitAll();
		static void ShutdownAll();

		static void DestroyAll();
	};
}