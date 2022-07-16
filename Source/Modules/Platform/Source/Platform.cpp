#include "System/System.h"
#include "Quartz.h"

#include "Platform.h"
#include "Application.h"
#include "Log.h"

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

namespace Quartz
{
	Application* spApp = nullptr;
}

extern "C"
{
	bool QUARTZ_API SystemQuery(bool isEditor, Quartz::SystemQueryInfo& systemQuery)
	{
		systemQuery.name = "PlatformModule";
		systemQuery.version = "1.0.0";

		return true;
	}

	bool QUARTZ_API SystemLoad(Log& engineLog)
	{
		Log::SetGlobalLog(engineLog);
		return true;
	}

	void QUARTZ_API SystemUnload()
	{

	}

	void QUARTZ_API SystemPreInit()
	{
		SystemInfo sysInfo = GenerateSystemInfo();

		ApplicationInfo appInfo = {};

		appInfo.appName     = "Quartz";
		appInfo.version     = "2.0.0";
		appInfo.windowApi   = WINDOW_API_GLFW;
		appInfo.logCallback = QuartzAppLogCallback;

		spApp = CreateApplication(appInfo);
	}

	void QUARTZ_API SystemInit()
	{
		WindowInfo		windowInfo		= { "Quartz Sandbox", 1280, 720, 100, 100, WINDOW_WINDOWED };
		//SurfaceInfo		surfaceInfo		= { SURFACE_API_VULKAN, &apiInfo };
		SurfaceInfo		surfaceInfo		= { SURFACE_API_NONE, nullptr };
		Window*			pWindow			= spApp->CreateWindow(windowInfo, surfaceInfo);

		spApp->SetWindowCloseRequestedCallback(WindowCloseRequestedCallback);
		spApp->SetWindowClosedCallback(WindowClosedCallback);
		spApp->SetWindowResizedCallback(WindowResizedCallback);
		spApp->SetWindowMovedCallback(WindowMovedCallback);
		spApp->SetWindowMaximizedCallback(WindowMaximizedCallback);
		spApp->SetWindowMinimizedCallback(WindowMinimizedCallback);
		spApp->SetWindowFocusedCallback(WindowFocusedCallback);
		spApp->SetKeyCallback(KeyCallback);
		spApp->SetKeyTypedCallback(KeyTypedCallback);
		spApp->SetMouseMovedCallback(MouseMovedCallback);
		spApp->SetMouseMovedRelativeCallback(MouseMovedRelativeCallback);
		spApp->SetMouseEnteredCallback(MouseEnteredCallback);
	}
}