#include "Engine.h"

#include "Debug.h"
#include "EngineAPI.h"
#include "Entity/World.h"
#include "Module/ModuleRegistry.h"
#include "Sinks/Windows/WinApiConsoleSink.h"
#include "Module/LibraryLoader.h"
#include "Resource/Loaders/ConfigHandler.h"

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
	Engine::mpGraphics;
	Engine::mpFilesystem;
	Engine::mpAssetManager;
	Engine::mpConfig;
	Engine::mpLog;
};

class GraphicsImpl : public Graphics
{
public:
	inline GraphicsImpl() : Graphics() {}
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

	/* Create Graphics */

	GraphicsImpl graphics;

	/////////////////////////////////////////////////////////////////////////////////

	/* Create Engine */

	EngineImpl engineImpl;
	engineImpl.mpWorld			= &world;
	engineImpl.mpRuntime		= &runtime;
	engineImpl.mpInput			= &input;
	engineImpl.mpDeviceRegistry = &deviceRegistry;
	engineImpl.mpModuleRegistry	= &moduleRegistry;
	engineImpl.mpGraphics		= &graphics;
	engineImpl.mpFilesystem		= &filesystem;
	engineImpl.mpAssetManager	= &assetManager;
	engineImpl.mpLog			= &engineLog;

	Engine::SetInstance(engineImpl);

	/////////////////////////////////////////////////////////////////////////////////

	/* Load + Pre-initialize Modules */

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
	ConfigHandler configHandler;
	assetManager.RegisterAssetHandler("ini", &configHandler);

	Config* pConfig = assetManager.GetOrLoadAsset<Config>("engine.ini");

	if (!pConfig)
	{
		LogError("Error loading engine configs: 'engine.ini' not found in root directory.");
		//LogInfo("'./engine.ini' created with default settings.");
	}

	engineImpl.mpConfig = pConfig;
	pConfig->PrintConfigs();

	/////////////////////////////////////////////////////////////////////////////////

	/* Initialize Modules */

	moduleRegistry.InitAll();

	/////////////////////////////////////////////////////////////////////////////////

	/* Start Graphics */

	const Array<String> apiNames = graphics.GetApiNames();

	DEBUG_ONLY
	{
		for (const String& apiName : apiNames)
		{
			LogDebug("Found Graphics Api [name=\"%s\"]", apiName.Str());
		}
	}

	if (apiNames.Size() == 0)
	{
		LogFatal("Engine configuration error: No graphics apis available.");
		return 1;
	}

	String graphicsApiName = apiNames[0];

	if (!pConfig->GetValue("graphicsApi", graphicsApiName))
	{
		LogWarning("Engine configuration warning: No graphics api set in \"engine.ini\". Using default.");
	}
	else if (!apiNames.Contains(graphicsApiName))
	{
		LogError("Engine configuration error: Unknown Graphics Api [name =\"%s\"]. Using default.", graphicsApiName.Str());
	}

	if (!graphics.Start(graphicsApiName))
	{
		LogFatal("graphics.Start() failed with Graphics Api [name=\"%s\"].", graphicsApiName.Str());
	}

	/////////////////////////////////////////////////////////////////////////////////

	/* Post-initialize Modules */

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