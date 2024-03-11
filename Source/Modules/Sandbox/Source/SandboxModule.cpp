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

#include "Physics.h"

#include <vulkan/vulkan.h>

#include "Input/Input.h"
//#include "Types/FractalGrid.h"

namespace Quartz
{
	extern "C"
	{
		using namespace Quartz;

		Window*			gpWindow;
		Entity			gpCamera;
		Entity			gpCube;
		Physics			gPhysics;

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

			//// TESTING ////

			//FractalGrid<uSize> fg;
			//
			//uSize val = 300;
			//
			//fg.Place(0, 20, 22, val);
			//
			//fg.Remove(0, 20, 22);
			//fg.Remove(6, 0, 0);

			/////////////////

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
					-0.5f, -0.5f, -0.5f,	1.0f, 0.0f, 0.0f,	 // 0 - Front Bottom Left
					-0.5f,  0.5f, -0.5f,	0.0f, 1.0f, 0.0f,	 // 1 - Front Top Left
					 0.5f,  0.5f, -0.5f,	0.0f, 0.0f, 1.0f,	 // 2 - Front Top Right
					 0.5f, -0.5f, -0.5f,	1.0f, 0.0f, 1.0f,	 // 3 - Front Bottom Right
					
					-0.5f, -0.5f,  0.5f,	1.0f, 1.0f, 0.0f,	 // 4 - Back Bottom Left
					-0.5f,  0.5f,  0.5f,	0.0f, 1.0f, 1.0f,	 // 5 - Back Top Left
					 0.5f,  0.5f,  0.5f,	1.0f, 1.0f, 1.0f,	 // 6 - Back Top Right
					 0.5f, -0.5f,  0.5f,	0.0f, 0.0f, 0.0f,	 // 7 - Back Bottom Right
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

			ModelData planeData
			{
				{
					-0.5f, 0.0f, -0.5f,		1.0f, 1.0f, 1.0f,	// Bottom Left
					-0.5f, 0.0f,  0.5f,		1.0f, 1.0f, 1.0f,	// Top Left
					 0.5f, 0.0f,  0.5f,		1.0f, 1.0f, 1.0f,	// Top Right
					 0.5f, 0.0f, -0.5f,		1.0f, 1.0f, 1.0f	// Bottom Right
				},
				{
					0, 1, 2, 0, 2, 3
				}
			};

			TransformComponent transformCube
			(
				{ 0.0f, 2.0f, 0.0f },
				Quatf({ 0.0f, 0.0f, 0.0f }, 0.0f),
				{ 1.0f, 1.0f, 1.0f }
			);

			TransformComponent transformTri
			( 
				{ -1.0f, -1.0f, -1.0f }, 
				Quatf({ 0.0f, 0.0f, 0.0f }, 0.0f), 
				{ 1.0f, 1.0f, 1.0f }
			);

			TransformComponent transformPlane
			(
				{ 0.0f, 0.0f, 0.0f },
				Quatf({ 0.0f, 0.0f, 0.0f }, 0.0f),
				{ 20.0f, 20.0f, 20.0f }
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

			MaterialComponent material3
			{
				"Shaders/default3.vert",
				"Shaders/default3.frag"
			};

			MeshComponent renderable1("simpleTri", triData);
			MeshComponent renderable2("simpleCube", cubeData);
			MeshComponent renderable3("simplePlane", planeData);
			//TerrainComponent terrainComponent;

			RigidBodyComponent cubePhysics;
			cubePhysics.friction = 1.0f;
			cubePhysics.collider = SphereCollider(transformCube, 0.5f);
			cubePhysics.AddForce(Vec3f(0.0f, -1.0f, 0.0f) * 0.0f);

			RigidBodyComponent planePhysics;
			planePhysics.friction = 1.0f;
			planePhysics.collider = PlaneCollider(transformPlane, { 0.0f, 1.0f, 0.0f }, 1.0f);

			RigidBodyComponent cameraPhysics;
			cameraPhysics.friction = 1.0f;
			cameraPhysics.collider = SphereCollider(transformCube, 0.5f);

			gpCube = world.CreateEntity(transformCube, renderable2, material1, cubePhysics);
			//Entity tri	= world.CreateEntity(transformTri, renderable1, material2);
			//Entity terr = world.CreateEntity(transformTerrain, renderable3, material3, terrainComponent);
			Entity plane = world.CreateEntity(transformPlane, renderable3, material1, planePhysics);

			CameraComponent camera(windowInfo.width, windowInfo.height, 70.0f, 0.001f, 1000.f);
			TransformComponent cameraTransform({ 0.0f, -2.0f, 0.0f }, { { 0.0f, 0.0f, 0.0f }, ToRadians(0.0f)}, {1.0f, 1.0f, 1.0f});
			gpCamera = world.CreateEntity(camera, cameraTransform, cameraPhysics);

			VulkanRenderer* pRenderer = new VulkanRenderer();
			pRenderer->Register(&runtime);
			pRenderer->Initialize(&gfx);
			pRenderer->SetCamera(gpCamera);

			runtime.SetTargetUps(350);
			runtime.SetTargetTps(20);

			runtime.RegisterOnUpdate(
				[](Runtime* pRuntime, double delta)
				{
					static double deltaAcc = 0;
					deltaAcc += delta;

					if (deltaAcc > 1.0)
					{
						deltaAcc = 0;
						//LogInfo("> FPS: %.1lf", pRuntime->GetCurrentUps());
					}

					TransformComponent& transform = Engine::GetWorld().Get<TransformComponent>(gpCamera);
					RigidBodyComponent& rigidBody = Engine::GetWorld().Get<RigidBodyComponent>(gpCamera);

					float speed = 2.0f;

					if (moveForward)
						//transform.position += transform.GetForward() * speed * delta;
						rigidBody.AddForce(transform.GetForward() * speed * delta);

					if (moveBackward)
						transform.position += transform.GetBackward() * speed * delta;

					if (moveLeft)
						transform.position += -transform.GetLeft() * speed * delta;

					if (moveRight)
						transform.position += -transform.GetRight() * speed * delta;

					/// PHYSICS

					gPhysics.Step(Engine::GetWorld(), delta);
					gPhysics.Resolve(Engine::GetWorld(), delta);
				}
			);

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
			input.MapKeyboardButton("Push",			INPUT_KEYBOARD_ANY, 33 /* > */, INPUT_ACTION_RELEASED);

			input.RegisterOnAxisInput("MouseLook",
				[](Vec2f direction, InputActions actions)
				{
					float upSpeed = 1.0f;
					float rightSpeed = 1.0f;

					Input& input = Engine::GetInput();
					Runtime& runtime = Engine::GetRuntime();

					if (captured)
					{
						TransformComponent& transform = Engine::GetWorld().Get<TransformComponent>(gpCamera);
						transform.rotation *= Quatf().SetAxisAngle({ 0.0f, 1.0f, 0.0f }, direction.x * upSpeed * runtime.GetUpdateDelta());
						transform.rotation *= Quatf().SetAxisAngle(transform.GetRight(), direction.y * rightSpeed * runtime.GetUpdateDelta());
					}
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
				
					captured = !captured;

					if (captured)
						LogSuccess("Capturing!");
					else
						LogSuccess("Uncapturing!");

					const InputMouse& mouse = *Engine::GetDeviceRegistry().GetPrimaryMouse();
					input.SetMouseHidden(*gpWindow, mouse, captured);
					input.SetMouseBounds(*gpWindow, mouse, gpWindow->GetBounds(), captured);
				}
			);

			input.RegisterOnButtonInput("Push",
				[](float value, InputActions actions)
				{
					RigidBodyComponent& physics = Engine::GetWorld().Get<RigidBodyComponent>(gpCube);
					TransformComponent& transform = Engine::GetWorld().Get<TransformComponent>(gpCamera);

					physics.AddForce(-transform.GetForward() * 1.0f);
				}
			);
		}

	}
}