#include "System/LibraryLoader.h"

#include "Windows/WinApiDynamicLibrary.h"
#include "Log.h"

namespace Quartz
{
	DynamicLibrary* LoadDynamicLibrary(const char* pathName)
	{
		LogInfo("Loading dynamic library: %s", pathName);

		HMODULE module = LoadLibraryEx(pathName, NULL, NULL);

		if (!module)
		{
			LogFatal("LoadLibraryEx() failed to load dynamic library: %s", pathName);
			WinApiPrintError();
			return nullptr;
		}

		return new WinApiDynamicLibrary(pathName, pathName, module);
	}

	bool FreeDynamicLibrary(DynamicLibrary* pLibrary)
	{
		WinApiDynamicLibrary* pWinApiDynamicLibrary = static_cast<WinApiDynamicLibrary*>(pLibrary);

		LogInfo("Freeing dynamic library: %s", pLibrary->GetName());

		if (!FreeLibrary(pWinApiDynamicLibrary->GetWinApiModuleHandle()))
		{
			LogError("FreeLibrary() failed to unload dynamic library: %s", pLibrary->GetPath().Str());
			WinApiPrintError();
			return false;
		}

		delete pWinApiDynamicLibrary;

		return true;
	}
}