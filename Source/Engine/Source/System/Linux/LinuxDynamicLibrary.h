#pragma once

#include "System/DynamicLibrary.h"

namespace Quartz
{
	class LinuxDynamicLibrary : public DynamicLibrary
	{
	private:
		void* mpLibrary;

	public:
		LinuxDynamicLibrary(const String& name, const String& path, void* pLibrary);

		void* GetFunction(const char* funcName) override;

		void* GetNativeHandle() override;
	};
}