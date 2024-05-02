#include "Module/Module.h"

#include "Engine.h"
#include "Entity/World.h"
#include "Input/Input.h"
#include "Runtime/Runtime.h"
#include "Runtime/Timer.h"
#include "Types/Array.h"
#include "Log.h"

#include "PlatformAPI.h"
#include "Platform.h"
#include "Application.h"
#include "Windows/RawInput.h"

#ifdef QUARTZAPP_VULKAN
#include <vulkan/vulkan.h>
#include "Vulkan/VulkanApiSurface.h"
#else
#include "Surface.h"
#endif

#include "Windows/WinApi.h"
#include "Windows/WinFilesystem.h"

#include <assert.h>

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

	Application*				gpApp;
	PlatformSingleton*			gpPlatform;
	RawInput					gRawInput;
	WinApiFilesystemHandler*	gpFilesystemHandler;

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
			case LOG_LEVEL_SUCCESS: LogSuccess(message); return;
			case LOG_LEVEL_FAIL:	LogFail(message);    return;
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
		Engine::GetRuntime().Stop();
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
		//LogTrace("[%s] %s", pWindow->GetTitle().Str(), restored ? "Maximization Restored" : "Maximized");
	}

	void WindowMinimizedCallback(Window* pWindow, bool restored)
	{
		//LogTrace("[%s] %s", pWindow->GetTitle().Str(), restored ? "Minimization Restored" : "Minimized");
	}

	void WindowFocusedCallback(Window* pWindow, bool lost)
	{
		//LogTrace("[%s] %s", pWindow->GetTitle().Str(), lost ? "Focus Lost" : "Focus Gained");
	}

	void KeyCallback(Window* pWindow, uInt16 scancode, bool down, bool repeat)
	{
		//LogTrace("KEY: %d %s", scancode, down ? (repeat ? "DOWN - REPEAT" : "DOWN") : "UP\n");
		InputActions actions = (down ? INPUT_ACTION_DOWN : INPUT_ACTION_UP) | (repeat ? INPUT_ACTION_REPEAT : 0);
		Engine::GetInput().SendButtonInput(INPUT_KEYBOARD_ANY, scancode, actions, 1.0f);
	}

	void KeyTypedCallback(Window* pWindow, char caracter, bool repeat)
	{
		//LogTrace("KEY-TYPED: %c %s", caracter, repeat ? "REP" : "");
	}

	void MouseMovedCallback(Window* pWindow, uSize mouseX, uSize mouseY)
	{
		//LogTrace("MOUSE MOVED: %d,%d", mouseX, mouseY);
	}

	void MouseMovedRelativeCallback(Window* pWindow, sSize relX, sSize relY)
	{
		//LogTrace("MOUSE MOVED: %d,%d - RELATIVE", relX, relY);
		//Engine::GetInput().SendAxisInput(INPUT_MOUSE_ANY, 0, INPUT_ACTION_MOVE, { (float)relX, (float)relY });
	}

	void MouseEnteredCallback(Window* pWindow, bool entered)
	{
		LogTrace("[%s] MOUSE %s", pWindow->GetTitle().Str(), entered ? "ENTERED" : "EXITED");
	}

	void Update(Runtime& runtime, double delta)
	{
		gRawInput.PollInput();
		gpApp->Update();
	}

	void Tick(Runtime& runtime, uSize tick)
	{
		if (tick == 0)
		{
			//gRawInput.PollConnections();
		}
	}
}

extern "C"
{
	using namespace Quartz;

	bool QUARTZ_PLATFORM_API ModuleQuery(bool isEditor, ModuleQueryInfo& moduleQuery)
	{
		moduleQuery.name = "PlatformModule";
		moduleQuery.version = "1.0.0";

		return true;
	}

	bool QUARTZ_PLATFORM_API ModuleLoad(Log& engineLog, Engine& engine)
	{
		Log::SetInstance(engineLog);
		Engine::SetInstance(engine);

		gpFilesystemHandler = new WinApiFilesystemHandler(nullptr);

		return true;
	}

	void QUARTZ_PLATFORM_API ModuleUnload()
	{
		DestroyApplication(gpApp);
		DestroyPlatform(gpPlatform);

		delete gpFilesystemHandler;
	}

	void QUARTZ_PLATFORM_API ModulePreInit()
	{
		Engine& engine = Engine::GetInstance();
		
		ApplicationInfo appInfo = {};

		appInfo.appName     = "Quartz";
		appInfo.version     = "2.0.0";
		appInfo.windowApi   = WINDOW_API_GLFW;
		appInfo.logCallback = QuartzAppLogCallback;

		gpApp = CreateApplication(appInfo);
		gpApp->UseRawInput(true);

		gpPlatform = &engine.GetWorld().CreateSingleton<PlatformSingleton>(); // @Deprecated
		CreatePlatform(gpApp, gpPlatform);

		gpPlatform->pApplication = gpApp;

		if (!engine.GetFilesystem().AddRoot(".", *gpFilesystemHandler, 1000))
		{
			LogFatal("Failed to read filesystem!");
			assert(false && "No filesystem!");
		}
	}

	void CreateDevices()
	{
		/*
		InputDeviceInfo genericMouseInfo = {};
		genericMouseInfo.deviceName		= L"Generic Mouse";
		genericMouseInfo.deviceVendor	= L"Unknown Vendor";
		genericMouseInfo.deviceType		= INPUT_DEVICE_TYPE_MOUSE;
		genericMouseInfo.buttonCount	= 2;
		genericMouseInfo.axisCount		= 1;

		InputDeviceCallbacks genericMouseCallbacks = {};

#ifdef _WIN32

		genericMouseCallbacks.onMouseSetPositionFunc =
			[](const InputDevice& device, Vec2i absPosition)
		{
			return (bool)SetCursorPos(absPosition.x, absPosition.y);
		};

		genericMouseCallbacks.onMouseGetPositionFunc = 
			[](const InputDevice& device, Vec2i& outAbsPosition)
		{
			POINT cursorPos = {};
			bool result = GetCursorPos(&cursorPos);
			outAbsPosition.x = cursorPos.x;
			outAbsPosition.y = cursorPos.y;
			return result;
		};

		genericMouseCallbacks.onMouseSetBoundsFunc =
			[](const Window& windowContext, const InputDevice& device, Bounds2i absBounds, bool enabled)
		{
			RECT clipRect = {};
			clipRect.left	= absBounds.BottomLeft().x;
			clipRect.right	= absBounds.TopRight().x;
			clipRect.top	= absBounds.BottomLeft().y;
			clipRect.bottom = absBounds.TopRight().y;
			bool result = ClipCursor(enabled ? &clipRect : NULL);
			return result;
		};

		genericMouseCallbacks.onMouseSetHiddenFunc =
			[](const Window& windowContext, const InputDevice& device, bool hidden)
		{
			ShowCursor(!hidden);
			return true;
		};

#endif

		Engine::GetDeviceRegistry().RegisterDevice(genericMouseInfo, genericMouseCallbacks);
		*/
	}

	void QUARTZ_PLATFORM_API ModuleInit()
	{
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

		gRawInput.Init();
		gRawInput.PollConnections();

		Engine::GetRuntime().RegisterOnUpdate(Update);
		Engine::GetRuntime().RegisterOnTick(Tick);
	}
}