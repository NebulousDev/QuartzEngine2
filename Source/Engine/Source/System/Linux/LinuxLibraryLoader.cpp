#include "System/LibraryLoader.h"

#include "System/Linux/LinuxDynamicLibrary.h"
#include "Log.h"

#include <dlfcn.h>

namespace Quartz
{
	DynamicLibrary* LoadDynamicLibrary(const char* pathName)
	{
		LogInfo("Loading dynamic library: %s", pathName);

		void* pLibrary = dlopen(pathName, RTLD_NOW);

		if (!pLibrary)
		{
			LogFatal("dlopen() failed to load dynamic library: %s", pathName);
			const char* pError = dlerror();
			LogError("Linux Error: %s", pError);
			return nullptr;
		}

		return new LinuxDynamicLibrary(pathName, pathName, pLibrary);
	}

	bool FreeDynamicLibrary(DynamicLibrary* pLibrary)
	{
		LinuxDynamicLibrary* pLinuxDynamicLibrary = static_cast<LinuxDynamicLibrary*>(pLibrary);

		LogInfo("Freeing dynamic library: %s", pLibrary->GetName());

		if (dlclose(pLibrary->GetNativeHandle()))
		{
			LogError("FreeLibrary() failed to unload dynamic library: %s", pLibrary->GetPath().Str());
			const char* pError = dlerror();
			LogError("Linux Error: %s", pError);
			return false;
		}

		delete pLibrary;

		return true;
	}
}