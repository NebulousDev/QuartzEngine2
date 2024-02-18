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

		virtual void* GetFunction(const char* funcName, bool optional) = 0;

		virtual void* GetNativeHandle() = 0;

		const String& GetName() const;
		const String& GetPath() const;
	};
}