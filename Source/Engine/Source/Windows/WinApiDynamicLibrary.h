#include "System/DynamicLibrary.h"

#include "WinApi.h"

namespace Quartz
{
	class WinApiDynamicLibrary : public DynamicLibrary
	{
	private:
		HMODULE mModule;

	public:
		WinApiDynamicLibrary(const String& name, const String& path, HMODULE module);

		void Load() override;
		void Unload() override;

		void* GetFunction(const char* funcName) override;

		void* GetNativeHandle() override;

		HMODULE GetWinApiModuleHandle();
	};
}