#pragma once

#include "EngineAPI.h"

#include "Entity/World.h"
#include "Input/Input.h"
#include "Runtime/Runtime.h"
#include "Input/InputDeviceRegistry.h"
#include "Module/ModuleRegistry.h"
#include "Filesystem/Filesystem.h"
#include "Resource/AssetManager.h"
#include "Config/Config.h"
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
		Filesystem*				mpFilesystem;
		AssetManager*			mpAssetManager;
		Config*					mpConfig;
		Log*					mpLog;

	public:
		static EntityWorld&			GetWorld();
		static Runtime&				GetRuntime();
		static Input&				GetInput();
		static InputDeviceRegistry& GetDeviceRegistry();
		static ModuleRegistry&		GetModuleRegistry();
		static Filesystem&			GetFilesystem();
		static AssetManager&		GetAssetManager();
		static Config&				GetConfig();
		static Log&					GetLog();

		static Engine&				GetInstance();
		static void					SetInstance(Engine& engine);
	};
}