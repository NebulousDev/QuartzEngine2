#include "System/DynamicLibrary.h"

namespace Quartz
{
	DynamicLibrary::DynamicLibrary(const String& name, const String& path)
		: mName(name), mPath(path) {}

	const String& DynamicLibrary::GetName() const
	{
		return mName;
	}

	const String& DynamicLibrary::GetPath() const
	{
		return mPath;
	}
}