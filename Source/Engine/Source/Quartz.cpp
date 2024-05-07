#include "Engine.h"

#include "EngineAPI.h"
#include "Entity/World.h"
#include "Module/ModuleRegistry.h"
#include "Sinks/Windows/WinApiConsoleSink.h"
#include "Module/LibraryLoader.h"
#include "Resource/Loaders/ConfigLoader.h"

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
	Engine::mpFilesystem;
	Engine::mpAssetManager;
	Engine::mpConfig;
	Engine::mpLog;
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

	/* Create Filesystem */

	Filesystem filesystem;

	/////////////////////////////////////////////////////////////////////////////////

	/* Create Asset Manager */

	AssetManager assetManager;

	/////////////////////////////////////////////////////////////////////////////////

	/* Create Runtime */

	Runtime runtime;

	/////////////////////////////////////////////////////////////////////////////////

	/* Create Input */

	Input input;
	InputDeviceRegistry deviceRegistry;

	/////////////////////////////////////////////////////////////////////////////////

	/* Create Module Admin */

	ModuleRegistry moduleRegistry;

	/////////////////////////////////////////////////////////////////////////////////

	/* Create Engine */

	EngineImpl engineImpl;
	engineImpl.mpWorld			= &world;
	engineImpl.mpRuntime		= &runtime;
	engineImpl.mpInput			= &input;
	engineImpl.mpDeviceRegistry = &deviceRegistry;
	engineImpl.mpModuleRegistry	= &moduleRegistry;
	engineImpl.mpFilesystem		= &filesystem;
	engineImpl.mpAssetManager	= &assetManager;
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

	/////////////////////////////////////////////////////////////////////////////////

	/* Setup Config */

	// We must set up configs here to allow module init functions access to engine.ini
	ConfigLoader configLoader;
	assetManager.RegisterAssetLoader("ini", &configLoader);

	Config* pConfig = assetManager.LoadAsset<Config>("engine.ini");
	engineImpl.mpConfig = pConfig;
	pConfig->PrintConfigs();

	/////////////////////////////////////////////////////////////////////////////////

	moduleRegistry.InitAll();
	moduleRegistry.PostInitAll();

	/////////////////////////////////////////////////////////////////////////////////

	/* Start */

	runtime.Start();

	/////////////////////////////////////////////////////////////////////////////////

	/* Shutdown */

	assetManager.UnloadAsset<Config>(pConfig);

	moduleRegistry.UnloadAll();
	moduleRegistry.DestroyAll();

	return 0;
}