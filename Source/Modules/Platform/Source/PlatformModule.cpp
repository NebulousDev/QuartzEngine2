#include "System/System.h"

#include "Quartz.h"
#include "Entity/World.h"
#include "Runtime/Runtime.h"
#include "Types/Array.h"
#include "Log.h"

#include "Platform.h"
#include "Application.h"

#ifdef QUARTZAPP_VULKAN
#include <vulkan/vulkan.h>
#include "Vulkan/VulkanApiSurface.h"
#else
#include "Surface.h"
#endif

namespace Quartz
{
	bool CreatePlatform(Application* pApplication, PlatformSingleton* pPlatform)
	{
		pPlatform->systemInfo = GenerateSystemInfo();
		pPlatform->pApplication = pApplication;
		return true;
	}

	void DestroyPlatform(PlatformSingleton* pPlatform)
	{

	}

	/////////////////////

	EntityWorld*		gpEntityWorld;
	Runtime*			gpRuntime;
	Application*		gpApp;
	PlatformSingleton*	gpPlatform;

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
		gpRuntime->Stop();
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

	void Update(Runtime* pRuntime, double delta)
	{
		gpApp->Update();
	}
}

extern "C"
{
	using namespace Quartz;

	bool QUARTZ_API SystemQuery(bool isEditor, Quartz::SystemQueryInfo& systemQuery)
	{
		systemQuery.name = "PlatformModule";
		systemQuery.version = "1.0.0";

		return true;
	}

	bool QUARTZ_API SystemLoad(Log& engineLog, EntityWorld& entityWorld, Runtime& runtime)
	{
		Log::SetGlobalLog(engineLog);
		gpEntityWorld = &entityWorld;
		gpRuntime = &runtime;
		return true;
	}

	void QUARTZ_API SystemUnload()
	{
		DestroyApplication(gpApp);
		DestroyPlatform(gpPlatform);
	}

	void QUARTZ_API SystemPreInit()
	{
		ApplicationInfo appInfo = {};

		appInfo.appName     = "Quartz";
		appInfo.version     = "2.0.0";
		appInfo.windowApi   = WINDOW_API_GLFW;
		appInfo.logCallback = QuartzAppLogCallback;

		gpApp = CreateApplication(appInfo);

		gpPlatform = &gpEntityWorld->CreateSingleton<PlatformSingleton>();
		CreatePlatform(gpApp, gpPlatform);

		gpPlatform->pApplication = gpApp;
	}

	void QUARTZ_API SystemInit()
	{
		/*
#ifdef QUARTZAPP_VULKAN

		VkSurfaceFormatKHR format = {};
		format.format		= VK_FORMAT_B8G8R8A8_SRGB;
		format.colorSpace	= VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;

		VulkanSurfaceInfo apiInfo = {};
		apiInfo.instance			= gpVulkanData->vkInstance;
		apiInfo.exclusiveFullscreen = false;
		apiInfo.physicalDevice		= gpVulkanData->primaryPhysicalDevice.vkPhysicalDevice;
		apiInfo.surfaceFormat		= format;

#endif

		WindowInfo		windowInfo		= { "Quartz Sandbox", 1280, 720, 100, 100, WINDOW_WINDOWED };
		SurfaceInfo		surfaceInfo		= { SURFACE_API_VULKAN, &apiInfo };
		Window*			pWindow			= gpApp->CreateWindow(windowInfo, surfaceInfo);

		gpApp->SetWindowCloseRequestedCallback(WindowCloseRequestedCallback); 
		gpApp->SetWindowClosedCallback(WindowClosedCallback);
		gpApp->SetWindowResizedCallback(WindowResizedCallback);
		gpApp->SetWindowMovedCallback(WindowMovedCallback);
		gpApp->SetWindowMaximizedCallback(WindowMaximizedCallback);
		gpApp->SetWindowMinimizedCallback(WindowMinimizedCallback);
		gpApp->SetWindowFocusedCallback(WindowFocusedCallback);
		gpApp->SetKeyCallback(KeyCallback);
		gpApp->SetKeyTypedCallback(KeyTypedCallback);
		gpApp->SetMouseMovedCallback(MouseMovedCallback);
		gpApp->SetMouseMovedRelativeCallback(MouseMovedRelativeCallback);
		gpApp->SetMouseEnteredCallback(MouseEnteredCallback);

		gpWindow = pWindow;

		//gpVulkanData->vkSurface = ((VulkanSurface*)pWindow->GetSurface())->GetVkSurface();
		*/

		gpRuntime->RegisterOnUpdate(Update);
	}
}