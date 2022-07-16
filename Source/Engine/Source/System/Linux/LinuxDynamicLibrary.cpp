#include "LinuxDynamicLibrary.h"

#include "Log.h"

#include <dlfcn.h>

namespace Quartz
{
	LinuxDynamicLibrary::LinuxDynamicLibrary(const String& name, const String& path, void* pLibrary)
		: DynamicLibrary(name, path), mpLibrary(pLibrary) { }

	void* LinuxDynamicLibrary::GetFunction(const char* funcName, bool optional)
	{
		void* func = dlsym(mpLibrary, funcName);

		if (!func && !optional)
		{
			LogError("dlsym() failed to retrieve function '%s' from module '%s'", funcName, mName.Str());
			return nullptr;
		}

		return func;
	}

	void* LinuxDynamicLibrary::GetNativeHandle()
	{
		return mpLibrary;
	}
}