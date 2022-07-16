#include "WinApiDynamicLibrary.h"

#include "Log.h"

namespace Quartz
{
	WinApiDynamicLibrary::WinApiDynamicLibrary(const String& name, const String& path, HMODULE module)
		: DynamicLibrary(name, path), mModule(module) { }

	void* WinApiDynamicLibrary::GetFunction(const char* funcName, bool optional)
	{
		FARPROC func = GetProcAddress(mModule, funcName);

		if (!func && !optional)
		{
			LogError("GetProcAddress() failed to retrieve function '%s' from module '%s'", funcName, mName.Str());
			WinApiPrintError();
			return nullptr;
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