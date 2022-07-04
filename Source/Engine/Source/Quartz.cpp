#include "stdio.h"
#include "Types/Map.h"
#include "Types/Set.h"
#include "Types/Stack.h"
#include "Types/String.h"
#include "Types/List.h"

#include "Math/Vector.h"

#include "Application.h"
#include "LogCallbacks.h"

#include "Log.h"

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

#ifdef QUARTZAPP_VULKAN
static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(
	VkDebugUtilsMessageSeverityFlagBitsEXT		messageSeverity,
	VkDebugUtilsMessageTypeFlagsEXT				messageType,
	const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
	void* pUserData)
{
	if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT)
	{
		//LogTrace("Vulkan: %s", pCallbackData->pMessage);
	}
	else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT)
	{
		//LogInfo("Vulkan: %s", pCallbackData->pMessage);
	}
	else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
	{
		LogWarning("Vulkan: %s", pCallbackData->pMessage);
	}
	else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
	{
		LogError("Vulkan: %s", pCallbackData->pMessage);
	}

	return VK_FALSE;
}
#endif

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

int main()
{
	Log::InitLogging();

	PrintBanner();

	Log::SetLogLevel(LOG_LEVEL_INFO);
	Log::RunLogTest();

#ifdef QUARTZENGINE_WINAPI
	DynamicLibrary* pGraphicsLibrary = LoadDynamicLibrary("GraphicsSystem.dll");
#elif defined QUARTZENGINE_LINUX
	DynamicLibrary* pGraphicsLibrary = LoadDynamicLibrary("libGraphicsSystem.so");
#endif

	System* pGraphicsSystem = SystemAdmin::CreateAndRegisterSystem(pGraphicsLibrary);

	SystemAdmin::LoadAll();

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

	/////////////////////////////////////////////////////////////////////////////////

#ifdef QUARTZAPP_VULKAN

	VkApplicationInfo vkAppInfo = {};
	vkAppInfo.apiVersion = VK_VERSION_1_2;
	vkAppInfo.pEngineName = "Quartz Engine 2";
	vkAppInfo.pApplicationName = "Quartz Sandbox";

	const char* extentions[] =
	{
		VK_EXT_DEBUG_UTILS_EXTENSION_NAME,
		VK_KHR_SURFACE_EXTENSION_NAME,
		"VK_KHR_win32_surface"
	};

	VkInstanceCreateInfo info = {};
	info.pApplicationInfo = &vkAppInfo;
	info.enabledExtensionCount = 3;
	info.ppEnabledExtensionNames = extentions;
	info.enabledLayerCount = 0;

	VkDebugUtilsMessengerCreateInfoEXT debugMessengerInfo = {};
	debugMessengerInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
	debugMessengerInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT
		| VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT
		| VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
	debugMessengerInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT
		| VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT
		| VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
	debugMessengerInfo.pfnUserCallback = DebugCallback;
	debugMessengerInfo.pUserData = NULL;
	debugMessengerInfo.pNext = NULL;

	info.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugMessengerInfo;

	VkInstance vkInstance;

	VkResult res = vkCreateInstance(&info, nullptr, &vkInstance);

	VkSurfaceFormatKHR format = {};
	format.format = VK_FORMAT_B8G8R8A8_SRGB;
	format.colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;

	VkResult result = VK_ERROR_UNKNOWN;
	uInt32 physicalDeviceCount = 0;
	Array<VkPhysicalDevice> physicalDevices;

	vkEnumeratePhysicalDevices(vkInstance, &physicalDeviceCount, nullptr);
	physicalDevices.Resize(physicalDeviceCount);
	vkEnumeratePhysicalDevices(vkInstance, &physicalDeviceCount, physicalDevices.Data());

	VulkanSurfaceInfo apiInfo = {};
	apiInfo.instance = vkInstance;
	apiInfo.exclusiveFullscreen = false;
	apiInfo.physicalDevice = physicalDevices[0];
	apiInfo.surfaceFormat = format;

#endif

	/////////////////////////////////////////////////////////////////////////////////

	ApplicationInfo appInfo		    = { WINDOW_API_GLFW, "Quartz", "2.0.0", QuartzAppLogCallback, 0};
	Application*	pApp		    = CreateApplication(appInfo);

	WindowInfo		windowInfo		= { "Quartz Sandbox", 1280, 720, 100, 100, WINDOW_WINDOWED };
	//SurfaceInfo		surfaceInfo		= { SURFACE_API_VULKAN, &apiInfo };
	SurfaceInfo		surfaceInfo		= { SURFACE_API_NONE, nullptr };
	Window*			pWindow			= pApp->CreateWindow(windowInfo, surfaceInfo);

	pApp->SetWindowCloseRequestedCallback(WindowCloseRequestedCallback);
	pApp->SetWindowClosedCallback(WindowClosedCallback);
	pApp->SetWindowResizedCallback(WindowResizedCallback);
	pApp->SetWindowMovedCallback(WindowMovedCallback);
	pApp->SetWindowMaximizedCallback(WindowMaximizedCallback);
	pApp->SetWindowMinimizedCallback(WindowMinimizedCallback);
	pApp->SetWindowFocusedCallback(WindowFocusedCallback);
	pApp->SetKeyCallback(KeyCallback);
	pApp->SetKeyTypedCallback(KeyTypedCallback);
	pApp->SetMouseMovedCallback(MouseMovedCallback);
	pApp->SetMouseMovedRelativeCallback(MouseMovedRelativeCallback);
	pApp->SetMouseEnteredCallback(MouseEnteredCallback);

	while(pWindow->IsOpen())
	{
		pApp->Update();
	}

	SystemAdmin::UnloadAll();
	SystemAdmin::DestroyAll();

	pApp->DestroyWindow(pWindow);

	DestroyApplication(pApp);

	return 0;
}