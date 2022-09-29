#include "Quartz.h"

#include "stdio.h"
#include "Types/Map.h"
#include "Types/Set.h"
#include "Types/Stack.h"
#include "Types/String.h"
#include "Types/List.h"

#include "Math/Vector.h"

#include "Application.h"
#include "LogCallbacks.h"

#include "Sinks/Windows/WinApiConsoleSink.h"

#include "Entity/World.h"
#include "System/SystemAdmin.h"
#include "System/LibraryLoader.h"

#include "Banner.h"

#include <cstdlib>
#include <vector>

#ifdef QUARTZAPP_VULKAN
#include <vulkan/vulkan.h>
#include "Vulkan/VulkanSurface.h"
#else
#include "Surface.h"
#endif

#include "Types/Tuple.h"

//#include "GLFW/glfw3.h"

using namespace Quartz;

void QuartzAppLogCallback(LogLevel level, const char* message)
{
	switch (level)
	{
		case LOG_LEVEL_TRACE:   LogTrace(message);   return;
		case LOG_LEVEL_DEBUG:   LogDebug(message);   return;
		case LOG_LEVEL_INFO:    LogInfo(message);    return;
		case LOG_LEVEL_WARNING: LogWarning(message); return;
		case LOG_LEVEL_ERROR:   LogError(message);   return;
		case LOG_LEVEL_FATAL:   LogFatal(message);   return;
	}
}

bool WindowCloseRequestedCallback(Window* pWindow)
{
	LogTrace("[%s] Requested Close", pWindow->GetTitle().Str());
	return true;
}

void WindowClosedCallback(Window* pWindow)
{
	LogTrace("[%s] Closed", pWindow->GetTitle().Str());
}

void WindowResizedCallback(Window* pWindow, uSize width, uSize height)
{
	//LogTrace("[%s] Resized - %dx%d", pWindow->GetTitle().Str(), width, height);
}

void WindowMovedCallback(Window* pWindow, uSize x, uSize y)
{
	//LogTrace("[%s] Moved - %dx%d", pWindow->GetTitle().Str(), x, y);
}

void WindowMaximizedCallback(Window* pWindow, bool restored)
{
	LogTrace("[%s] %s", pWindow->GetTitle().Str(), restored ? "Maximization Restored" : "Maximized");
}

void WindowMinimizedCallback(Window* pWindow, bool restored)
{
	LogTrace("[%s] %s", pWindow->GetTitle().Str(), restored ? "Minimization Restored" : "Minimized");
}

void WindowFocusedCallback(Window* pWindow, bool lost)
{
	LogTrace("[%s] %s", pWindow->GetTitle().Str(), lost ? "Focus Lost" : "Focus Gained");
}

void KeyCallback(Window* pWindow, uInt16 scancode, bool down, bool repeat)
{
	LogTrace("KEY: %d %s", scancode, down ? (repeat ? "DOWN - REPEAT" : "DOWN") : "UP\n");
}

void KeyTypedCallback(Window* pWindow, char caracter, bool repeat)
{
	//LogTrace("KEY-TYPED: %c %s", caracter, repeat ? "REP" : "");
}

void MouseMovedCallback(Window* pWindow, uSize mouseX, uSize mouseY)
{
	//LogTrace("MOUSE MOVED: %d,%d", mouseX, mouseY);
}

void MouseMovedRelativeCallback(Window* pWindow, uSize relX, uSize relY)
{
	//LogTrace("MOUSE MOVED: %d,%d - RELATIVE", relX, relY);
}

void MouseEnteredCallback(Window* pWindow, bool entered)
{
	//LogTrace("[%s] MOUSE %s", pWindow->GetTitle().Str(), entered ? "ENTERED" : "EXITED");
}

#include <Windows.h>
#include <cstdio>
#include <fcntl.h>
#include <time.h>
#include <io.h>

#undef CreateWindow

struct ControlSingleton
{
	bool running;
};

int main()
{
	WinApiConsoleSink winConsoleSink;
	winConsoleSink.SetLogLevel(LOG_LEVEL_TRACE);

	Log engineLog = Log({&winConsoleSink});
	Log::SetGlobalLog(engineLog);

	PrintBanner();

	//engineLog.SetLogLevel(LOG_LEVEL_ERROR);
	engineLog.RunLogTest();

	/////////////////////////////////////////////////////////////////////////////////

	EntityDatabase database;
	EntityGraph graph(&database);
	EntityWorld world(&database, &graph);

	Entity e1 = world.CreateEntity(TransformComponent({ 5,5,5 }, Quatf(), { 1,1,1 }));

	for (Entity& e : world.CreateView<TransformComponent>())
	{
		TransformComponent& t = world.GetComponent<TransformComponent>(e);
		int i = 5;
	}

	ControlSingleton& control = world.CreateSingleton<ControlSingleton>();
	control.running = true;

	/////////////////////////////////////////////////////////////////////////////////

	Runtime runtime;

	/////////////////////////////////////////////////////////////////////////////////

#ifdef QUARTZENGINE_WINAPI 
	DynamicLibrary* pPlatformLibrary = LoadDynamicLibrary("Platform.dll");
	DynamicLibrary* pGraphicsLibrary = LoadDynamicLibrary("Graphics.dll");
#elif defined QUARTZENGINE_LINUX
	DynamicLibrary* pPlatformLibrary = LoadDynamicLibrary("libPlatform.so");
	DynamicLibrary* pGraphicsLibrary = LoadDynamicLibrary("libGraphics.so");
#endif

	System* pPlatformSystem = SystemAdmin::CreateAndRegisterSystem(pPlatformLibrary);
	System* pGraphicsSystem = SystemAdmin::CreateAndRegisterSystem(pGraphicsLibrary);

	SystemAdmin::LoadAll(engineLog, world, runtime);
	SystemAdmin::PreInitAll();
	SystemAdmin::InitAll();
	SystemAdmin::PostInitAll();

	while(control.running)
	{
		runtime.UpdateAll();
	}

	SystemAdmin::UnloadAll();
	SystemAdmin::DestroyAll();

	return 0;
}