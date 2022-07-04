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

	bool FreeDynamicLibrary(DynamicLibrary* pDynamicLibrary)
	{
		WinApiDynamicLibrary* pWinApiDynamicLibrary = static_cast<WinApiDynamicLibrary*>(pDynamicLibrary);

		LogInfo("Freeing dynamic library: %s", pDynamicLibrary->GetName());

		if (!FreeLibrary(pWinApiDynamicLibrary->GetWinApiModuleHandle()))
		{
			LogError("FreeLibrary() failed to unload dynamic library: %s", pDynamicLibrary->GetPath().Str());
			WinApiPrintError();
			return false;
		}

		delete pWinApiDynamicLibrary;

		return true;
	}
}