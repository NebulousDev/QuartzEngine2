#pragma once

#include "Types/String.h"

namespace Quartz
{
	class DynamicLibrary
	{
	protected:
		String mName;
		String mPath;

	public:
		DynamicLibrary(const String& name, const String& path);

		virtual void Load() = 0;
		virtual void Unload() = 0;

		virtual void* GetFunction(const char* funcName) = 0;

		virtual void* GetNativeHandle() = 0;

		const String& GetName() const;
		const String& GetPath() const;
	};
}