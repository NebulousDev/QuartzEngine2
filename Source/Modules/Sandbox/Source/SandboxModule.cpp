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
#include "Component/TerrainComponent.h"

#include "Physics.h"

#include <vulkan/vulkan.h>

#include "Input/Input.h"
#include "Filesystem/File.h"
#include "Filesystem/Folder.h"
#include "Memory/Allocator.h"
#include "Utility/StringReader.h"

#include "Resource/Loaders/ModelHandler.h"
#include "Resource/Loaders/ImageHandler.h"
#include "Resource/Binary/QModelParser.h"

#include "Runtime/Timer.h"
#include "Utility/RefCounter.h"

namespace Quartz
{
	extern "C"
	{
		using namespace Quartz;

		Window*			gpWindow;
		Entity			gCamera;
		Entity			gEntity0;
		Entity			gEntity1;
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

		bool moveSpeed		= false;
		bool superMoveSpeed = false;

		bool captured		= false;

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

		ModelData planeData2
		{
			{
				-0.5f, 0.0f, -0.5f,		0.5f, 0.5f, 0.5f,	// Bottom Left
				-0.5f, 0.0f,  0.5f,		0.5f, 0.5f, 0.5f,	// Top Left
				 0.5f, 0.0f,  0.5f,		0.5f, 0.5f, 0.5f,	// Top Right
				 0.5f, 0.0f, -0.5f,		0.5f, 0.5f, 0.5f	// Bottom Right
			},
			{
				0, 1, 2, 0, 2, 3
			}
		};

		TransformComponent transformCube
		(
			{ 0.0f, 10.0f, 0.0f },
			Quatf({ 1.0f, 1.0f, 0.0f }, ToRadians(0.0f)),
			{ 1.0f, 1.0f, 1.0f }
		);

		TransformComponent transformCube2
		(
			{ 0.0f, 15.0f, 0.0f },
			Quatf({ 1.0f, 0.0f, 0.0f }, ToRadians(0.0f)),
			{ 2.0f, 2.0f, 2.0f }
		);

		TransformComponent transformPlane
		(
			{ 0.0f, 10.0f, 0.0f },
			Quatf({ 0.0f, 0.0f, 0.0f }, 0.0f),
			{ 20.0f, 1.0f, 20.0f }
		);

		TransformComponent transformPlaneL
		(
			{ -10.0f, 10.0f, 0.0f },
			Quatf({ 0.0f, 0.0f, 1.0f }, ToRadians(90.0f)),
			{ 20.0f, 1.0f, 20.0f }
		);

		TransformComponent transformPlaneR
		(
			{ 10.0f, 10.0f, 0.0f },
			Quatf({ 0.0f, 0.0f, 1.0f }, ToRadians(-90.0f)),// * Quatf({ 0.0f, 1.0f, 0.0f }, ToRadians(180.0f)),
			{ 20.0f, 1.0f, 20.0f }
		);

		TransformComponent transformPlaneF
		(
			{ 0.0f, 10.0f, -10.0f },
			Quatf({ 1.0f, 0.0f, 0.0f }, ToRadians(-90.0f)),// * Quatf({ 1.0f, 0.0f, 0.0f }, ToRadians(-90.0f)),
			{ 20.0f, 1.0f, 20.0f }
		);

		TransformComponent transformPlaneB
		(
			{ 0.0f, 10.0f, 10.0f },
			Quatf({ 1.0f, 0.0f, 0.0f }, ToRadians(90.0f)),// * Quatf({ 1.0f, 0.0f, 0.0f }, ToRadians(90.0f)),
			{ 20.0f, 1.0f, 20.0f }
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

		RigidBody cubeRigidBody(0.1f, 0.6f, 1.0f, { 0.0f, 0.0f, 0.0f });
		RectCollider cubeCollider(Bounds3f{ {-0.5f, -0.5f, -0.5f}, {0.5f, 0.5f, 0.5f} }, false);
		//SphereCollider cubeCollider(0.5f, false);

		RigidBody cubeRigidBody2(0.1f, 0.6f, 1.0f);
		RectCollider cubeCollider2(Bounds3f{ {-0.5f, -0.5f, -0.5f}, {0.5f, 0.5f, 0.5f} }, false);
		//SphereCollider cubeCollider2(0.5f, false);

		RigidBody planeRigidBody(0.0f, 1.0f, 1.0f, { 0.0f, 0.0f, 0.0f });
		PlaneCollider planeCollider({ 0.0f, 1.0f, 0.0f }, 0.0f, true);

		RigidBody cameraRigidBody(0.0f, 1.0f, 1.0f, { 0.0f, 0.0f, 0.0f });
		SphereCollider cameraCollider(0.5f, true);

		void QuickConvertToQMF(const String& objFilePath, const String& qmfFilePath)
		{
			File* pQMFFile = Engine::GetFilesystem().CreateFile(qmfFilePath);
			Model* pModel = Engine::GetAssetManager().GetOrLoadAsset<Model>(objFilePath);
			QModelParser qmfWriter(*pQMFFile);
			qmfWriter.SetModel(*pModel);
			qmfWriter.Write();
		}

		void QUARTZ_ENGINE_API ModulePostInit()
		{
			ModelHandler* pModelHandler = new ModelHandler; // TODO
			Engine::GetAssetManager().RegisterAssetHandler("obj", pModelHandler);
			Engine::GetAssetManager().RegisterAssetHandler("qmodel", pModelHandler);

			ImageHandler* pImageHandler = new ImageHandler;
			Engine::GetAssetManager().RegisterAssetHandler("png", pImageHandler);
			Engine::GetAssetManager().RegisterAssetHandler("jpeg", pImageHandler);
			Engine::GetAssetManager().RegisterAssetHandler("bmp", pImageHandler);
			Engine::GetAssetManager().RegisterAssetHandler("gif", pImageHandler);
			Engine::GetAssetManager().RegisterAssetHandler("hdr", pImageHandler);

			//File* pQMFFile = Engine::GetFilesystem().CreateFile("Assets/Models/dva.qmod");
			//Model* pModel = Engine::GetAssetManager().GetOrLoadAsset<Model>("Assets/Models/dva.obj");
			//
			//QModelParser qmfWriter(*pQMFFile);
			//
			//qmfWriter.SetModel(*pModel);
			//qmfWriter.Write();
			//
			//PoolAllocator<Model> modelAllocs(200);
			//PoolAllocator<ByteBuffer> bufferAllocs(200);
			//QModelParser qmfReader(*pQMFFile, &modelAllocs, &bufferAllocs);
			//
			//qmfReader.Read();
			//
			//Model* pModelRead = qmfReader.GetModel();

			//Folder* pFolder = Engine::GetFilesystem().GetFolder("Assets/Models");
			//
			//for (File* pFile : pFolder->GetFiles())
			//{
			//	if (pFile->GetExtention() == "obj"_STR)
			//	{
			//		String objPath = pFile->GetPath().Substring(2);
			//		String qModPath = String(pFolder->GetPath().Substring(2)) + "/" +
			//			pFile->GetName().Substring(0, pFile->GetName().Find(".obj")) + ".qmod";
			//		QuickConvertToQMF(objPath, qModPath);
			//	}
			//}

			//QuickConvertToQMF("Assets/Models/sibenik.obj", "Assets/Models/sibenik.qmod");
			//QuickConvertToQMF("Assets/Models/sponza.obj", "Assets/Models/sponza.qmod");
			//QuickConvertToQMF("Assets/Models/testScene.obj", "Assets/Models/testScene.qmod");

			RigidBodyComponent cubePhysics(cubeRigidBody, cubeCollider);
			RigidBodyComponent cubePhysics2(cubeRigidBody2, cubeCollider2);
			RigidBodyComponent planePhysics(planeRigidBody, planeCollider);
			RigidBodyComponent planePhysics2(planeRigidBody, planeCollider);
			RigidBodyComponent cameraPhysics(cameraRigidBody, cameraCollider);

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

			gPhysics.Initialize();

			Image* pImage = Engine::GetAssetManager().GetOrLoadAsset<Image>("Assets/Textures/default.png");

			//transformCube2.scale /= 100.0f;

			gEntity0 = world.CreateEntity(transformCube, MeshComponent("Assets/Models/bunny.qmodel"), material1);
			//gEntity1 = world.CreateEntity(transformCube, MeshComponent("Assets/Models/cube.qmodel"), material1, cubePhysics2);
			Entity gEntity2 = world.CreateEntity(transformCube2, MeshComponent("Assets/Models/sibenik.qmodel"), material1);
			Entity plane = world.CreateEntity(transformPlane, material1, planePhysics);

			Entity planeL = world.CreateEntity(transformPlaneL, material1, planePhysics2);
			Entity planeR = world.CreateEntity(transformPlaneR, material1, planePhysics2);
			Entity planeF = world.CreateEntity(transformPlaneF, material1, planePhysics2);
			Entity planeB = world.CreateEntity(transformPlaneB, material1, planePhysics2);

			TerrainSettings terrainSettings;
			terrainSettings.resolution		= 200;
			terrainSettings.seed			= 1234;
			terrainSettings.scale			= 550.0f;
			terrainSettings.lacunarity		= 1.5f;
			terrainSettings.octaveWeights	= { 1.0f, 0.3f, 0.15f, 0.10f, 0.10f, 0.05f };

			Entity terrain = world.CreateEntity(TerrainComponent(terrainSettings));

			CameraComponent camera(windowInfo.width, windowInfo.height, 70.0f, 0.001f, 1000.f);
			TransformComponent cameraTransform({ 0.0f, 12.0f, 4.0f }, { { 0.0f, 1.0f, 0.0f }, ToRadians(0.0f)}, {1.0f, 1.0f, 1.0f});
			gCamera = world.CreateEntity(camera, cameraTransform, cameraPhysics);

			uSize maxInFlightCount = 3;
			VulkanRenderer* pRenderer = new VulkanRenderer(gfx, *gfx.pPrimaryDevice, *gpWindow, maxInFlightCount);

			pRenderer->Initialize();
			pRenderer->Register(runtime);
			pRenderer->SetCamera(gCamera);
			pRenderer->SetTargetFPS(350);
			//pRenderer->SetTargetFPS(5000);

			//runtime.SetTargetUps(350);
			runtime.SetTargetUps(5000);
			runtime.SetTargetTps(60);

			runtime.RegisterOnUpdate(
				[](Runtime& runtime, double delta)
				{
					static double deltaAcc = 0;
					deltaAcc += delta;

					if (deltaAcc > 1.0)
					{
						deltaAcc = 0;
						//LogInfo("> FPS: %.1lf", runtime.GetCurrentUps());
					}

					TransformComponent& cameraTransform = Engine::GetWorld().Get<TransformComponent>(gCamera);
					RigidBodyComponent& cameraRigidBody = Engine::GetWorld().Get<RigidBodyComponent>(gCamera);

					float speed = superMoveSpeed ? 200.0f : (moveSpeed ? 20.0f : 2.0f);

					if (moveForward)
						cameraTransform.position -= cameraTransform.GetForward() * speed * delta;

					if (moveBackward)
						cameraTransform.position -= cameraTransform.GetBackward() * speed * delta;

					if (moveLeft)
						cameraTransform.position += cameraTransform.GetLeft() * speed * delta;

					if (moveRight)
						cameraTransform.position += cameraTransform.GetRight() * speed * delta;

					cameraTransform.rotation.Normalize();

					/// PHYSICS

					gPhysics.Step(Engine::GetWorld(), delta);
				}
			);

			runtime.RegisterOnTick(
				[](Runtime& runtime, uSize tick)
				{
					//gPhysics.Step(Engine::GetWorld(), 1.0 / runtime.GetTargetTps());
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

			input.MapKeyboardButton("MoveSpeed",		INPUT_KEYBOARD_ANY, 42 /* SHIFT */, INPUT_ACTION_ANY);
			input.MapKeyboardButton("SuperMoveSpeed",	INPUT_KEYBOARD_ANY, 34 /* G */,		INPUT_ACTION_ANY);

			input.MapKeyboardButton("Interact",		INPUT_KEYBOARD_ANY, 18 /* E */, INPUT_ACTION_RELEASED);
			input.MapKeyboardButton("Push",			INPUT_KEYBOARD_ANY, 33 /* F */, INPUT_ACTION_RELEASED);

			input.RegisterOnAxisInput("MouseLook",
				[](Vec2f direction, InputActions actions)
				{
					float upSpeed = 1.0f;
					float rightSpeed = 1.0f;

					Input& input = Engine::GetInput();
					Runtime& runtime = Engine::GetRuntime();

					if (captured)
					{
						TransformComponent& transform = Engine::GetWorld().Get<TransformComponent>(gCamera);

						Quatf rotX = Quatf().SetAxisAngle(Vec3f::UP, (double)direction.x * (double)upSpeed * 0.002f);// *runtime.GetUpdateDelta());
						Quatf rotY = Quatf().SetAxisAngle(transform.GetRight(), (double)direction.y * (double)rightSpeed * 0.002f);// *runtime.GetUpdateDelta());

						transform.rotation *= rotX;
	
						Vec3f lookDir = rotY * transform.GetForward();
						Vec3f backDir = Cross(Vec3f::UP, transform.GetRight());
						
						float dot = Dot(lookDir, backDir);

						if (dot <= 0.0f)
						{
							transform.rotation *= rotY;  // Could be better
						}
					}
				}
			);

			input.RegisterOnButtonInput("MoveForward",
				[](float value, InputActions actions) { moveForward = actions & INPUT_ACTION_DOWN; }
			);

			input.RegisterOnButtonInput("MoveSpeed",
				[](float value, InputActions actions) { moveSpeed = actions & INPUT_ACTION_DOWN; }
			);

			input.RegisterOnButtonInput("SuperMoveSpeed",
				[](float value, InputActions actions) { superMoveSpeed = actions & INPUT_ACTION_DOWN; }
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
					TransformComponent& cameraTransform = Engine::GetWorld().Get<TransformComponent>(gCamera);
					Vec3f normal = -cameraTransform.GetForward();

					TransformComponent transform
					(
						cameraTransform.position + normal * 2.0f,
						cameraTransform.rotation,
						{ 1.0f, 1.0f, 1.0f }
					);

					RigidBody projectileBody(0.1f, 0.6f, 1.0f);
					//RectCollider projectileCollider(Bounds3f{ {-0.5f, -0.5f, -0.5f}, {0.5f, 0.5f, 0.5f} }, false);
					SphereCollider projectileCollider(1.0f, false);
					RigidBodyComponent projectilePhysics(projectileBody, projectileCollider);

					projectilePhysics.AddForce(normal * 100000.0f);
					projectilePhysics.AddTorque({ 0.0f, 1.0f, 0.0f });

					Entity projectileEntity = Engine::GetWorld().CreateEntity(
						transform, 
						MeshComponent("Assets/Models/bunny.obj"), 
						material1, 
						projectilePhysics);

				}
			);
		}

	}
}