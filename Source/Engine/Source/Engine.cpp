#include "Engine.h"

namespace Quartz
{
	Engine* Engine::spInstance = nullptr;

	EntityWorld& Engine::GetWorld()
	{
		return *spInstance->mpWorld;
	}

	Runtime& Engine::GetRuntime()
	{
		return *spInstance->mpRuntime;
	}

	Input& Engine::GetInput()
	{
		return *spInstance->mpInput;
	}

	InputDeviceRegistry& Engine::GetDeviceRegistry()
	{
		return *spInstance->mpDeviceRegistry;
	}

	ModuleRegistry& Engine::GetModuleRegistry()
	{
		return *spInstance->mpModuleRegistry;
	}

	Log& Engine::GetLog()
	{
		return *spInstance->mpLog;
	}

	Engine& Engine::GetInstance()
	{
		return *spInstance;
	}
	void Engine::SetInstance(Engine& engine)
	{
		spInstance = &engine;
	}
}