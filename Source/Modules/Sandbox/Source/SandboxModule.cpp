#include "Module/Module.h"

#include "Engine.h"
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

#include "Input/Input.h"

namespace Quartz
{
	extern "C"
	{
		using namespace Quartz;

		Window*			gpWindow;
		Entity			gpCamera;

		bool QUARTZ_ENGINE_API ModuleQuery(bool isEditor, Quartz::ModuleQueryInfo& moduleQuery)
		{
			moduleQuery.name = "SandboxModule";
			moduleQuery.version = "1.0.0";

			return true;
		}

		bool QUARTZ_ENGINE_API ModuleLoad(Log& engineLog, Engine& engine)
		{
			Log::SetInstance(engineLog);
			Engine::SetInstance(engine);

			return true;
		}

		void QUARTZ_ENGINE_API ModuleUnload()
		{

		}

		void QUARTZ_ENGINE_API ModulePreInit()
		{

		}

		struct TestEvent
		{
			uInt32 value;
		};

		bool moveForward	= false;
		bool moveBackward	= false;
		bool moveLeft		= false;
		bool moveRight		= false;

		bool captured		= false;

		void QUARTZ_ENGINE_API ModuleInit()
		{
			LogInfo("Starting Sandbox");

			StartVulkan();

			EntityWorld& world	= Engine::GetWorld();
			Input& input		= Engine::GetInput();
			Runtime& runtime	= Engine::GetRuntime();

			// TODO: Transition entity module to pointers
			PlatformSingleton& platform = world.Get<PlatformSingleton>();
			VulkanGraphics&    gfx      = world.Get<VulkanGraphics>();

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
							gpWindow		= platform.pApplication->CreateWindow(windowInfo, surfaceInfo);

			// TEMP
			gfx.pSurface = gfx.pResourceManager->CreateSurface(gfx.pPrimaryDevice, gfx.vkInstance, *(VulkanApiSurface*)gpWindow->GetSurface());

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
					0, 7, 4,  0, 3, 7,		// Bottom

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

			Entity cube = world.CreateEntity(transform0, renderable2, material1);
			Entity tri	= world.CreateEntity(transform1, renderable1, material2);

			CameraComponent camera(70.0f, 0.001f, 1000.f);
			TransformComponent cameraTransform({ 0.0f, 0.0f, -2.0f }, { { 0.0f, 0.0f, 0.0f }, 0.0f }, { 1.0f, 1.0f, 1.0f });
			gpCamera = world.CreateEntity(camera, cameraTransform);

			VulkanRenderer* pRenderer = new VulkanRenderer();
			pRenderer->Register(&runtime);
			pRenderer->Initialize(&gfx);
			pRenderer->SetCamera(gpCamera);

			runtime.SetTargetUps(350);

			runtime.RegisterOnUpdate(
				[](Runtime* pRuntime, double delta)
				{
					static double deltaAcc = 0;
					deltaAcc += delta;

					if (deltaAcc > 1.0)
					{
						deltaAcc = 0;
						LogInfo("> FPS: %.1lf", pRuntime->GetCurrentUps());
					}

					TransformComponent& transform = Engine::GetWorld().Get<TransformComponent>(gpCamera);

					float speed = 1.0f;

					if(moveForward)
						transform.position += transform.GetForward() * speed * delta;

					if (moveBackward)
						transform.position += transform.GetBackward() * speed * delta;

					if (moveLeft)
						transform.position += transform.GetLeft() * speed * delta;

					if (moveRight)
						transform.position += transform.GetRight() * speed * delta;
				}
			);

			runtime.RegisterOnEvent<TestEvent>(
				[](Runtime* pRuntime, const TestEvent& event)
				{
					LogSuccess("Lambdas!");
				}
			);

			runtime.Trigger(TestEvent{}, false);

			input.MapMouseAxis("MouseLook",			INPUT_MOUSE_ANY,				INPUT_ACTION_MOVE);

			input.MapKeyboardButton("MoveForward",	INPUT_KEYBOARD_ANY, 17 /* W */, INPUT_ACTION_ANY);
			input.MapKeyboardButton("MoveForward",	INPUT_KEYBOARD_ANY, 72 /* ^ */, INPUT_ACTION_ANY);
			input.MapKeyboardButton("MoveBackward",	INPUT_KEYBOARD_ANY, 31 /* S */, INPUT_ACTION_ANY);
			input.MapKeyboardButton("MoveBackward",	INPUT_KEYBOARD_ANY, 80 /* v */, INPUT_ACTION_ANY);
			input.MapKeyboardButton("MoveLeft",		INPUT_KEYBOARD_ANY, 30 /* A */, INPUT_ACTION_ANY);
			input.MapKeyboardButton("MoveLeft",		INPUT_KEYBOARD_ANY, 75 /* < */, INPUT_ACTION_ANY);
			input.MapKeyboardButton("MoveRight",	INPUT_KEYBOARD_ANY, 32 /* D */, INPUT_ACTION_ANY);
			input.MapKeyboardButton("MoveRight",	INPUT_KEYBOARD_ANY, 77 /* > */, INPUT_ACTION_ANY);

			input.MapKeyboardButton("Interact",		INPUT_KEYBOARD_ANY, 18 /* E */, INPUT_ACTION_RELEASED);

			input.RegisterOnAxisInput("MouseLook",
				[](Vec2f direction, InputActions actions)
				{
					float upSpeed = 1.0f;
					float rightSpeed = 1.0f;

					Input& input = Engine::GetInput();
					Runtime& runtime = Engine::GetRuntime();

					TransformComponent& transform = Engine::GetWorld().Get<TransformComponent>(gpCamera);
					transform.rotation *= Quatf().SetAxisAngle({ 0.0f, 1.0f, 0.0f }, direction.x * upSpeed * runtime.GetUpdateDelta());
					transform.rotation *= Quatf().SetAxisAngle(transform.GetRight(), direction.y * -rightSpeed * runtime.GetUpdateDelta());
				}
			);

			input.RegisterOnButtonInput("MoveForward",
				[](float value, InputActions actions) { moveForward = actions & INPUT_ACTION_DOWN; }
			);

			input.RegisterOnButtonInput("MoveBackward",
				[](float value, InputActions actions) { moveBackward = actions & INPUT_ACTION_DOWN; }
			);

			input.RegisterOnButtonInput("MoveLeft",
				[](float value, InputActions actions) { moveLeft = actions & INPUT_ACTION_DOWN; }
			);

			input.RegisterOnButtonInput("MoveRight",
				[](float value, InputActions actions) { moveRight = actions & INPUT_ACTION_DOWN; }
			);

			input.RegisterOnButtonInput("Interact",
				[](float value, InputActions actions)
				{
					Input& input = Engine::GetInput();
				
					LogSuccess("Capturing!");

					captured = !captured;

					const InputMouse& mouse = Engine::GetDeviceRegistry().GetDevices()[0];
					//input.SetMouseHidden(*gpWindow, mouse, captured);
					input.SetMouseBounds(*gpWindow, mouse, gpWindow->GetBounds(), captured);
				}
			);
		}

	}
}