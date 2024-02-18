#include "Engine.h"

#include "EngineAPI.h"
#include "Sinks/Windows/WinApiConsoleSink.h"
#include "Module/LibraryLoader.h"

#include "Banner.h"

using namespace Quartz;

class EngineImpl : public Engine
{
public:
	Engine::mpWorld;
	Engine::mpRuntime;
	Engine::mpInput;
	Engine::mpDeviceRegistry;
	Engine::mpModuleRegistry;
	Engine::mpLog;

	inline void SetWorld(EntityWorld* pWorld) { mpWorld = pWorld; }
	inline void SetRuntime(Runtime* pRuntime) { mpRuntime = pRuntime; }
	inline void SetInput(Input* pInput) { mpInput = pInput; }
	inline void SetDeviceRegistry(InputDeviceRegistry* pDeviceRegistry) { mpDeviceRegistry = pDeviceRegistry; }
	inline void SetModuleRegistry(ModuleRegistry* pModuleRegistry) { mpModuleRegistry = pModuleRegistry; }
	inline void SetLog(Log* pLog) { mpLog = pLog; }
};

int main()
{
	/////////////////////////////////////////////////////////////////////////////////

	/* Setup Engine Logging */

#ifdef QUARTZENGINE_WINAPI 
	WinApiConsoleSink winConsoleSink;
	winConsoleSink.SetLogLevel(LOG_LEVEL_TRACE);
	Log engineLog = Log({&winConsoleSink});
#elif defined QUARTZENGINE_LINUX
	Log engineLog = Log({});
#endif
	
	Log::SetInstance(engineLog);

	PrintBanner();

	engineLog.RunLogTest();

	/////////////////////////////////////////////////////////////////////////////////

	/* Create Entity World */

	EntityDatabase database;
	EntityGraph graph(&database);
	EntityWorld world(&database, &graph);

	/////////////////////////////////////////////////////////////////////////////////

	/* Create Runtime */

	Runtime& runtime						= world.CreateSingleton<Runtime>();

	/////////////////////////////////////////////////////////////////////////////////

	/* Create Input */

	Input& input							= world.CreateSingleton<Input>();
	InputDeviceRegistry& deviceRegistry		= world.CreateSingleton<InputDeviceRegistry>();

	/////////////////////////////////////////////////////////////////////////////////

	/* Create Module Admin */

	ModuleRegistry& moduleRegistry			= world.CreateSingleton<ModuleRegistry>();

	/////////////////////////////////////////////////////////////////////////////////

	/* Create Engine */

	EngineImpl engineImpl;
	engineImpl.mpWorld			= &world;
	engineImpl.mpRuntime		= &runtime;
	engineImpl.mpInput			= &input;
	engineImpl.mpDeviceRegistry = &deviceRegistry;
	engineImpl.mpModuleRegistry	= &moduleRegistry;
	engineImpl.mpLog			= &engineLog;

	Engine::SetInstance(engineImpl);

	/////////////////////////////////////////////////////////////////////////////////

	/* Load Modules */

#ifdef QUARTZENGINE_WINAPI 
	DynamicLibrary* pPlatformLibrary = LoadDynamicLibrary("Platform.dll");
	DynamicLibrary* pGraphicsLibrary = LoadDynamicLibrary("Graphics.dll");
	DynamicLibrary* pSandboxLibrary  = LoadDynamicLibrary("Sandbox.dll");
#elif defined QUARTZENGINE_LINUX
	DynamicLibrary* pPlatformLibrary = LoadDynamicLibrary("libPlatform.so");
	DynamicLibrary* pGraphicsLibrary = LoadDynamicLibrary("libGraphics.so");
	DynamicLibrary* pSandboxLibrary  = LoadDynamicLibrary("libSandbox.so");
#endif

	Module* pPlatformModule = moduleRegistry.CreateAndRegisterModule(pPlatformLibrary);
	Module* pGraphicsModule = moduleRegistry.CreateAndRegisterModule(pGraphicsLibrary);
	Module* pSandboxModule  = moduleRegistry.CreateAndRegisterModule(pSandboxLibrary);

	moduleRegistry.LoadAll(engineLog, Engine::GetInstance());
	moduleRegistry.PreInitAll();
	moduleRegistry.InitAll();
	moduleRegistry.PostInitAll();

	/////////////////////////////////////////////////////////////////////////////////

	/* Start */

	runtime.Start();

	/////////////////////////////////////////////////////////////////////////////////

	/* Shutdown */

	moduleRegistry.UnloadAll();
	moduleRegistry.DestroyAll();

	return 0;
}