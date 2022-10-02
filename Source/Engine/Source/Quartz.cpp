#include "Quartz.h"

#include "Sinks/Windows/WinApiConsoleSink.h"

#include "Entity/World.h"
#include "System/SystemAdmin.h"
#include "System/LibraryLoader.h"

#include "Banner.h"

using namespace Quartz;

struct TestTrigger
{
	int a = 0;
};

Runtime* gpRuntime;

void OnTestTrigger(const TestTrigger& trigger)
{
	LogTrace(">> (CORE) TestTrigger:%d <<", trigger.a);
	gpRuntime->UnregisterOnTrigger<TestTrigger>(OnTestTrigger);
}


void OnTestTick(uSize tick)
{
	if(tick == 0)
		gpRuntime->Trigger<TestTrigger>(TestTrigger{ 5 });
}

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
	
	Log::SetGlobalLog(engineLog);

	PrintBanner();

	engineLog.RunLogTest();

	/////////////////////////////////////////////////////////////////////////////////

	/* Create Entity Databases */

	EntityDatabase database;
	EntityGraph graph(&database);
	EntityWorld world(&database, &graph);

	/////////////////////////////////////////////////////////////////////////////////

	/* Create Runtime */

	Runtime runtime;
	gpRuntime = &runtime;

	runtime.RegisterTriggerType<TestTrigger>();
	runtime.RegisterOnTrigger<TestTrigger>(OnTestTrigger);
	runtime.RegisterOnTick(OnTestTick);

	/////////////////////////////////////////////////////////////////////////////////

	/* Load Systems */

#ifdef QUARTZENGINE_WINAPI 
	//DynamicLibrary* pPlatformLibrary = LoadDynamicLibrary("Platform.dll");
	DynamicLibrary* pGraphicsLibrary = LoadDynamicLibrary("Graphics.dll");
#elif defined QUARTZENGINE_LINUX
	DynamicLibrary* pPlatformLibrary = LoadDynamicLibrary("libPlatform.so");
	DynamicLibrary* pGraphicsLibrary = LoadDynamicLibrary("libGraphics.so");
#endif

	//System* pPlatformSystem = SystemAdmin::CreateAndRegisterSystem(pPlatformLibrary);
	System* pGraphicsSystem = SystemAdmin::CreateAndRegisterSystem(pGraphicsLibrary);

	SystemAdmin::LoadAll(engineLog, world, runtime);
	SystemAdmin::PreInitAll();
	SystemAdmin::InitAll();
	SystemAdmin::PostInitAll();

	/////////////////////////////////////////////////////////////////////////////////

	/* Start */

	runtime.Start();

	/////////////////////////////////////////////////////////////////////////////////

	/* Shutdown */

	SystemAdmin::UnloadAll();
	SystemAdmin::DestroyAll();

	return 0;
}