#pragma once

#include "EngineAPI.h"
#include "Entity/World.h"
#include "Input/Input.h"
#include "Runtime/Runtime.h"
#include "Input/InputDeviceRegistry.h"
#include "Module/ModuleRegistry.h"
#include "Log.h"

namespace Quartz
{
	class QUARTZ_ENGINE_API Engine
	{
	protected:
		static Engine*			spInstance;

		EntityWorld*			mpWorld;
		Runtime*				mpRuntime;
		Input*					mpInput;
		InputDeviceRegistry*	mpDeviceRegistry;
		ModuleRegistry*			mpModuleRegistry;
		Log*					mpLog;

	public:
		static inline EntityWorld&			GetWorld() { return *spInstance->mpWorld; }
		static inline Runtime&				GetRuntime() { return *spInstance->mpRuntime; }
		static inline Input&				GetInput() { return *spInstance->mpInput; }
		static inline InputDeviceRegistry&	GetDeviceRegistry() { return *spInstance->mpDeviceRegistry; }
		static inline ModuleRegistry&		GetModuleRegistry() { return *spInstance->mpModuleRegistry; }
		static inline Log&					GetLog() { return *spInstance->mpLog; }

		static inline Engine&				GetInstance() { return *spInstance; }
		static inline void					SetInstance(Engine& engine) { spInstance = &engine; }
	};

	inline Engine* Engine::spInstance = nullptr;
}