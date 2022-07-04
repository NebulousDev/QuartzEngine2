#include "WinApiDynamicLibrary.h"

#include "Log.h"

namespace Quartz
{
	WinApiDynamicLibrary::WinApiDynamicLibrary(const String& name, const String& path, HMODULE module)
		: DynamicLibrary(name, path), mModule(module) { }

	void WinApiDynamicLibrary::Load()
	{

	}

	void WinApiDynamicLibrary::Unload()
	{

	}

	void* WinApiDynamicLibrary::GetFunction(const char* funcName)
	{
		FARPROC func = GetProcAddress(mModule, funcName);

		if (!func)
		{
			LogError("GetProcAddress() failed to retrieve function '%s' from module '%s'", funcName, mName.Str());
			WinApiPrintError();
		}

		return func;
	}

	void* WinApiDynamicLibrary::GetNativeHandle()
	{
		return (void*)mModule;
	}

	HMODULE WinApiDynamicLibrary::GetWinApiModuleHandle()
	{
		return mModule;
	}
}