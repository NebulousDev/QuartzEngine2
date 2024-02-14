#include "System/System.h"

#include "EngineAPI.h"
#include "Entity/World.h"
#include "Log.h"

#include "Platform.h"
#include "Graphics.h"

#include "Vulkan/VulkanGraphics.h"
#include "Vulkan/Primatives/VulkanSurface.h"
#include "Vulkan/VulkanApiSurface.h"
#include "Vulkan/VulkanRenderer.h"

#include "Component/TransformComponent.h"
#include "Component/MeshComponent.h"
#include "Component/MaterialComponent.h"
#include "Component/CameraComponent.h"

#include <vulkan/vulkan.h>

namespace Quartz
{
	extern "C"
	{
		using namespace Quartz;

		EntityWorld*	gpWorld;
		Runtime*		gpRuntime;

		Entity			gpCamera;

		bool QUARTZ_ENGINE_API SystemQuery(bool isEditor, Quartz::SystemQueryInfo& systemQuery)
		{
			systemQuery.name = "SandboxModule";
			systemQuery.version = "1.0.0";

			return true;
		}

		bool QUARTZ_ENGINE_API SystemLoad(Log& engineLog, EntityWorld& entityWorld, Runtime& runtime)
		{
			Log::SetGlobalLog(engineLog);

			gpWorld = &entityWorld;
			gpRuntime = &runtime;

			return true;
		}

		void QUARTZ_ENGINE_API SystemUnload()
		{

		}

		void QUARTZ_ENGINE_API SystemPreInit()
		{

		}

		double deltaAcc;

		void OnUpdate(Runtime* pRuntime, double delta)
		{
			deltaAcc += delta;

			if (deltaAcc > 1.0)
			{
				deltaAcc = 0;
				LogInfo("> FPS: %.1lf", pRuntime->GetCurrentUps());
			}

			TransformComponent& transform = gpWorld->Get<TransformComponent>(gpCamera);
			transform.rotation *= Quatf().SetAxisAngle({ 0.0f, 1.0f, 0.0f }, 1.0f * delta);
		}

		struct TestEvent
		{
			uInt32 value;
		};

		void QUARTZ_ENGINE_API SystemInit()
		{
			LogInfo("Starting Sandbox");

			StartVulkan();

			// TODO: Transition entity system to pointers
			PlatformSingleton& platform = gpWorld->Get<PlatformSingleton>();
			VulkanGraphics&    gfx      = gpWorld->Get<VulkanGraphics>();

			VkSurfaceFormatKHR format = {};
			format.format		= VK_FORMAT_B8G8R8A8_SRGB;
			format.colorSpace	= VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;

			VulkanApiSurfaceInfo apiInfo = {};
			apiInfo.instance			= gfx.vkInstance;
			apiInfo.exclusiveFullscreen = false;
			apiInfo.physicalDevice		= gfx.primaryPhysicalDevice.vkPhysicalDevice;
			apiInfo.surfaceFormat		= format;

			WindowInfo		windowInfo		= { "QuartzEngine 2 - Sandbox", 1280, 720, 100, 100, WINDOW_WINDOWED };
			SurfaceInfo		surfaceInfo		= { SURFACE_API_VULKAN, &apiInfo };
			Window*			pWindow			= platform.pApplication->CreateWindow(windowInfo, surfaceInfo);

			// TEMP
			gfx.pSurface = gfx.pResourceManager->CreateSurface(gfx.pPrimaryDevice, gfx.vkInstance, *(VulkanApiSurface*)pWindow->GetSurface());

			ModelData triData
			{
				{
					-0.5f, -0.5f, 0.0f,  1.0f, 0.0f, 0.0f,
					 0.0f,  0.5f, 0.0f,  0.0f, 1.0f, 0.0f,
					 0.5f, -0.5f, 0.0f,  0.0f, 0.0f, 1.0f
				},
				{
					0, 1, 2
				}
			};

			ModelData cubeData
			{
				{
					-0.5f, -0.5f,  0.5f,	1.0f, 0.0f, 0.0f,	 // 0 - Front Bottom Left
					-0.5f,  0.5f,  0.5f,	0.0f, 1.0f, 0.0f,	 // 1 - Front Top Left
					 0.5f,  0.5f,  0.5f,	0.0f, 0.0f, 1.0f,	 // 2 - Front Top Right
					 0.5f, -0.5f,  0.5f,	1.0f, 0.0f, 1.0f,	 // 3 - Front Bottom Right
					
					-0.5f, -0.5f, -0.5f,	1.0f, 1.0f, 0.0f,	 // 4 - Back Bottom Left
					-0.5f,  0.5f, -0.5f,	0.0f, 1.0f, 1.0f,	 // 5 - Back Top Left
					 0.5f,  0.5f, -0.5f,	1.0f, 1.0f, 1.0f,	 // 6 - Back Top Right
					 0.5f, -0.5f, -0.5f,	0.0f, 0.0f, 0.0f,	 // 7 - Back Bottom Right
				},
				{
					0, 1, 2,  0, 2, 3,		// Front
					3, 2, 6,  3, 6, 7,		// Right
					7, 6, 5,  7, 5, 4,		// Back
					4, 5, 1,  4, 1, 0,		// Left
					1, 5, 6,  1, 6, 2,		// Top
					0, 5, 6,  0, 6, 3,		// Bottom

				}
			};

			TransformComponent transform0
			(
				{ -1.0f, 0.0f, -1.0f },
				Quatf({ 0.0f, 1.0f, 0.0f }, ToRadians(45.0f)),
				{ 1.0f, 1.0f, 1.0f }
			);

			TransformComponent transform1
			( 
				{ 1.0f, 0.0f, -1.0f }, 
				Quatf({ 0.0f, 0.0f, 0.0f }, 0.0f), 
				{ 1.0f, 1.0f, 1.0f }
			);

			MaterialComponent material1
			{
				"Shaders/default.vert",
				"Shaders/default.frag"
			};

			MaterialComponent material2
			{
				"Shaders/default2.vert",
				"Shaders/default2.frag"
			};

			MeshComponent renderable1("simpleTri", triData);
			MeshComponent renderable2("simpleCube", cubeData);

			Entity cube = gpWorld->CreateEntity(transform0, renderable2, material1);
			Entity tri	= gpWorld->CreateEntity(transform1, renderable1, material2);

			CameraComponent camera(70.0f, 0.001f, 1000.f);
			TransformComponent cameraTransform({ 0.0f, 0.0f, -2.0f }, { { 0.0f, 0.0f, 0.0f }, 0.0f }, { 1.0f, 1.0f, 1.0f });
			gpCamera = gpWorld->CreateEntity(camera, cameraTransform);

			VulkanRenderer* pRenderer = new VulkanRenderer();
			pRenderer->Register(gpRuntime);
			pRenderer->Initialize(&gfx);
			pRenderer->SetCamera(gpCamera);

			gpRuntime->SetTargetUps(350);
			gpRuntime->RegisterOnUpdate(OnUpdate);

			gpRuntime->RegisterOnEvent<TestEvent>(
				[](Runtime* pRuntime, const TestEvent& event)
				{
					LogSuccess("Lambdas!");
				}
			);

			gpRuntime->Trigger(TestEvent{}, false);
		}

	}
}