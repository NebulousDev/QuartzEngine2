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
#include "Component/LightComponent.h"
//#include "Component/TerrainComponent.h"

#include "Physics.h"

#include <vulkan/vulkan.h>

#include "Input/Input.h"
#include "Filesystem/File.h"
#include "Filesystem/Folder.h"
#include "Memory/Allocator.h"
#include "Utility/StringReader.h"

#include "Resource/Loaders/ModelHandler.h"
#include "Resource/Loaders/ImageHandler.h"
#include "Resource/Loaders/NativeShaderHandler.h"
#include "Resource/Loaders/ShaderHandler.h"
#include "Resource/Loaders/MaterialHandler.h"
#include "Resource/Binary/QModelParser.h"
#include "Resource/Binary/QShaderParser.h"

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

		MaterialComponent testMaterial
		{
			{
				"Assets/Materials/material_worn_stone.qmaterial",
				"Assets/Materials/material_polished_tile.qmaterial",
				"Assets/Materials/material_gold.qmaterial",
				"Assets/Materials/material_treebark.qmaterial",
				"Assets/Materials/material_marble.qmaterial",
				"Assets/Materials/material_worn_stone.qmaterial",
				"Assets/Materials/material_worn_stone.qmaterial",
				"Assets/Materials/material_wood.qmaterial",
				"Assets/Materials/material_worn_stone.qmaterial",
				"Assets/Materials/material_gold.qmaterial",
				"Assets/Materials/material_worn_stone.qmaterial",
				"Assets/Materials/material_gold.qmaterial",
				"Assets/Materials/material_wood.qmaterial",
				"Assets/Materials/material_wood.qmaterial",
				"Assets/Materials/material_gold.qmaterial",
				"Assets/Materials/material_gold.qmaterial",
				"Assets/Materials/material_wood.qmaterial",
				"Assets/Materials/material_gold.qmaterial",
				"Assets/Materials/material_gold.qmaterial",
				"Assets/Materials/material_worn_stone.qmaterial",
				"Assets/Materials/material_worn_stone.qmaterial",
				"Assets/Materials/material_worn_stone.qmaterial",
				"Assets/Materials/material_worn_stone.qmaterial"
			}
		};

		MaterialComponent gunMaterial
		{
			{
				"Assets/Materials/gun/material_gun.qmaterial"
			}
		};

		MaterialComponent testSceneMaterial
		{
			{
				"Assets/Materials/material_wood.qmaterial"
			}
		};

		MaterialComponent goldMaterial
		{
			{
				"Assets/Materials/material_gold.qmaterial"
			}
		};

		MaterialComponent marbleMaterial
		{
			{
				"Assets/Materials/material_marble.qmaterial"
			}
		};

		MaterialComponent treebarkMaterial
		{
			{
				"Assets/Materials/material_treebark.qmaterial"
			}
		};

		MaterialComponent polishedMaterial
		{
			{
				"Assets/Materials/material_polished_tile.qmaterial"
			}
		};

		MaterialComponent scifiMaterial
		{
			{
				"Assets/Materials/material_space_cruiser.qmaterial"
			}
		};

		MaterialComponent wornStoneMaterial
		{
			{
				"Assets/Materials/material_worn_stone.qmaterial"
			}
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

		void QuickConvertToQModel(const String& objFilePath, const String& qModelFilePath)
		{
			File* pQMFFile = Engine::GetFilesystem().CreateFile(qModelFilePath);
			Model* pModel = Engine::GetAssetManager().GetOrLoadAsset<Model>(objFilePath);
			QModelParser qmfWriter(*pQMFFile);
			qmfWriter.SetModel(*pModel);
			qmfWriter.Write();
		}

		void QuckConvertToQShader(const String& glslFilePath, const String& qShaderFilePath)
		{
			File* pShaderFile = Engine::GetFilesystem().CreateFile(qShaderFilePath);
			Shader* pShader = Engine::GetAssetManager().GetOrLoadAsset<Shader>(glslFilePath);
			QShaderParser qShaderWriter(*pShaderFile);
			qShaderWriter.SetShader(*pShader);
			qShaderWriter.Write();
		}

		void QUARTZ_ENGINE_API ModulePostInit()
		{
			ModelHandler* pModelHandler = new ModelHandler; // TODO
			Engine::GetAssetManager().RegisterAssetHandler("obj", pModelHandler);
			Engine::GetAssetManager().RegisterAssetHandler("qmodel", pModelHandler);

			ImageHandler* pImageHandler = new ImageHandler;
			Engine::GetAssetManager().RegisterAssetHandler("png", pImageHandler);
			Engine::GetAssetManager().RegisterAssetHandler("jpeg", pImageHandler);
			Engine::GetAssetManager().RegisterAssetHandler("jpg", pImageHandler);
			Engine::GetAssetManager().RegisterAssetHandler("bmp", pImageHandler);
			Engine::GetAssetManager().RegisterAssetHandler("gif", pImageHandler);
			Engine::GetAssetManager().RegisterAssetHandler("hdr", pImageHandler);
			Engine::GetAssetManager().RegisterAssetHandler("tga", pImageHandler);

			NativeShaderHandler* pNativeShaderHandler = new NativeShaderHandler;
			Engine::GetAssetManager().RegisterAssetHandler("vert", pNativeShaderHandler);
			Engine::GetAssetManager().RegisterAssetHandler("frag", pNativeShaderHandler);

			ShaderHandler* pShaderHandler = new ShaderHandler;
			Engine::GetAssetManager().RegisterAssetHandler("qsvert", pShaderHandler);
			Engine::GetAssetManager().RegisterAssetHandler("qsfrag", pShaderHandler);

			MaterialHandler* pMaterialHandler = new MaterialHandler;
			Engine::GetAssetManager().RegisterAssetHandler("qmaterial", pMaterialHandler);

			//QuckConvertToQShader("Shaders/visualize_normal.frag", "Shaders/visualize_normal.qsfrag");
			//QuckConvertToQShader("Shaders/visualize_normal.vert", "Shaders/visualize_normal.qsvert");
			//QuckConvertToQShader("Shaders/default2.frag", "Shaders/default2.qsfrag");
			//QuckConvertToQShader("Shaders/default2.vert", "Shaders/default2.qsvert");
			//QuckConvertToQShader("Shaders/default3.frag", "Shaders/default3.qsfrag");
			//QuckConvertToQShader("Shaders/default3.vert", "Shaders/default3.qsvert");
			//QuckConvertToQShader("Shaders/fullscreen.vert", "Shaders/fullscreen.qsvert");
			//QuckConvertToQShader("Shaders/skyScatterLUT.frag", "Shaders/skyScatterLUT.qsfrag");
			//QuckConvertToQShader("Shaders/skyTransmittanceLUT.frag", "Shaders/skyTransmittanceLUT.qsfrag");
			//QuckConvertToQShader("Shaders/skyViewLUT.frag", "Shaders/skyViewLUT.qsfrag");
			//QuckConvertToQShader("Shaders/sky.frag", "Shaders/sky.qsfrag");
			//QuckConvertToQShader("Shaders/terrain.vert", "Shaders/terrain.qsvert");
			//QuckConvertToQShader("Shaders/terrain.frag", "Shaders/terrain.qsfrag");

			//QuckConvertToQShader("Shaders/basic_mesh.vert", "Shaders/basic_mesh.qsvert");
			//QuckConvertToQShader("Shaders/basic_color.frag", "Shaders/basic_color.qsfrag");
			//QuckConvertToQShader("Shaders/basic_texture.frag", "Shaders/basic_texture.qsfrag");

			//QuckConvertToQShader("Shaders/pbr_rma.frag", "Shaders/Dva/pbr_rma.qsfrag");
			//QuckConvertToQShader("Shaders/pbr_rma_separate.frag", "Shaders/pbr_rma_separate.qsfrag");

			//QuckConvertToQShader("Shaders/tonemap_hdr-sdr.frag", "Shaders/tonemap_hdr-sdr.qsfrag");

			//Folder* pFolder = Engine::GetFilesystem().GetFolder("Assets/Models");
			
			//for (File* pFile : pFolder->GetFiles())
			//{
			//	if (pFile->GetExtention() == "obj"_STR)
			//	{
			//		String objPath = pFile->GetPath().Substring(2);
			//		String qModPath = String(pFolder->GetPath().Substring(2)) + "/" +
			//			pFile->GetName().Substring(0, pFile->GetName().Find(".obj")) + ".qmodel";
			//		QuickConvertToQModel(objPath, qModPath);
			//	}
			//}

			//QuickConvertToQMF("Assets/Models/sibenik.obj", "Assets/Models/sibenik.qmod");
			//QuickConvertToQModel("Assets/Models/dva.obj", "Assets/Models/dva.qmodel");
			 
			//QuickConvertToQModel("Assets/Models/bistro.obj", "Assets/Models/bistro.qmodel");
			//QuickConvertToQModel("Assets/Models/bmw.obj", "Assets/Models/bmw.qmodel");
			//QuickConvertToQModel("Assets/Models/bunny.obj", "Assets/Models/bunny.qmodel");
			//QuickConvertToQModel("Assets/Models/bunny_no_normal.obj", "Assets/Models/bunny_no_normal.qmodel");
			//QuickConvertToQModel("Assets/Models/cube.obj", "Assets/Models/cube.qmodel");
			//QuickConvertToQModel("Assets/Models/cube_mat.obj", "Assets/Models/cube_mat.qmodel");
			//QuickConvertToQModel("Assets/Models/dragon.obj", "Assets/Models/dragon.qmodel");
			//QuickConvertToQModel("Assets/Models/rei.obj", "Assets/Models/rei.qmodel");
			//QuickConvertToQModel("Assets/Models/sibenik.obj", "Assets/Models/sibenik.qmodel");
			//QuickConvertToQModel("Assets/Models/sponza.obj", "Assets/Models/sponza.qmodel");
			//QuickConvertToQModel("Assets/Models/testScene.obj", "Assets/Models/testScene.qmodel");
			//QuickConvertToQModel("Assets/Models/gun.obj", "Assets/Models/gun.qmodel");

			//int j = 25;

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

			//WindowInfo		windowInfo		= { "QuartzEngine 2 - Sandbox", 1280, 720, 100, 100, WINDOW_WINDOWED };
			WindowInfo		windowInfo		= { "QuartzEngine 2 - Sandbox", 1920, 1080, 100, 100, WINDOW_WINDOWED };
			//WindowInfo		windowInfo		= { "QuartzEngine 2 - Sandbox", 2560, 1440, 0, 0, WINDOW_FULLSCREEN };
			SurfaceInfo		surfaceInfo		= { SURFACE_API_VULKAN, &apiInfo };
							gpWindow		= platform.pApplication->CreateWindow(windowInfo, surfaceInfo);

			// TEMP
			gfx.pSurface = gfx.pResourceManager->CreateSurface(gfx.pPrimaryDevice, gfx.vkInstance, *(VulkanApiSurface*)gpWindow->GetSurface());

			gPhysics.Initialize();

			TransformComponent transform0
			(
				{ 0.0f, 10.0f, 0.0f },
				Quatf({ 1.0f, 1.0f, 0.0f }, ToRadians(0.0f)),
				{ 1.0f, 1.0f, 1.0f }
			);

			TransformComponent transform1
			(
				{ -0.5f, 10.0f, 0.0f },
				Quatf({ 1.0f, 0.0f, 0.0f }, ToRadians(0.0f)),
				{ 1.0f, 1.0f, 1.0f }
			);

			TransformComponent transform2
			(
				{ 0.2f, 10.08f, 0.0f }, 
				Quatf({ 0.0f, 1.0f, 0.0f }, ToRadians(-90.0f)),
				{ 1.0f, 1.0f, 1.0f }
			);

			TransformComponent transform3
			(
				{ -0.2f, 9.5f, 0.0f },
				Quatf({ 0.0f, 1.0f, 0.0f }, ToRadians(-90.0f)),
				{ 1.0f, 1.0f, 1.0f }
			);

			transform0.scale /= 1000.0f;
			//transform1.scale /= 100.0f;
			transform1.scale /= 12.0f;
			transform2.scale /= 8.0f;

			Entity entity0 = world.CreateEntity(transform0, MeshComponent("Assets/Models/sponza.qmodel"), testMaterial);
			Entity entity1 = world.CreateEntity(transform1, MeshComponent("Assets/Models/bmw.qmodel"), scifiMaterial);
			Entity entity2 = world.CreateEntity(transform2, MeshComponent("Assets/Models/gun.qmodel"), gunMaterial);
			Entity entity3 = world.CreateEntity(transform3, MeshComponent("Assets/Models/testScene.qmodel"), testSceneMaterial);

			TransformComponent lightTransform0
			(
				{ 0.5f, 10.2f, 0.0f },
				Quatf({ 1.0f, 1.0f, 0.0f }, ToRadians(0.0f)),
				{ 1.0f, 1.0f, 1.0f }
			);

			TransformComponent lightTransform1
			(
				{ 0.0f, 10.2f, 0.0f },
				Quatf({ 1.0f, 1.0f, 0.0f }, ToRadians(0.0f)),
				{ 1.0f, 1.0f, 1.0f }
			);

			TransformComponent lightTransform2
			(
				{ -0.5f, 10.2f, 0.0f },
				Quatf({ 1.0f, 1.0f, 0.0f }, ToRadians(0.0f)),
				{ 1.0f, 1.0f, 1.0f }
			);

			TransformComponent lightTransform3
			(
				{ 1.0f, 15.0f, 2.0f },
				Quatf({ 1.0f, 1.0f, 0.0f }, ToRadians(0.0f)),
				{ 1.0f, 1.0f, 1.0f }
			);

			Entity light0 = world.CreateEntity(lightTransform0, LightComponent({ {1,0,0}, 0.5 }));
			Entity light1 = world.CreateEntity(lightTransform1, LightComponent({ {0,1,0}, 0.5 }));
			Entity light2 = world.CreateEntity(lightTransform2, LightComponent({ {0,0,1}, 0.5 }));

			Entity light3 = world.CreateEntity(lightTransform3, LightComponent({ {1,1,1}, 100.0 }));

			//Entity plane = world.CreateEntity(transformPlane, testMaterial, planePhysics);
			//Entity planeL = world.CreateEntity(transformPlaneL, testMaterial, planePhysics2);
			//Entity planeR = world.CreateEntity(transformPlaneR, testMaterial, planePhysics2);
			//Entity planeF = world.CreateEntity(transformPlaneF, testMaterial, planePhysics2);
			//Entity planeB = world.CreateEntity(transformPlaneB, testMaterial, planePhysics2);
			
			//gEntity0 = entity0;
			//gEntity1 = entity1;

			//TerrainSettings terrainSettings;
			//terrainSettings.resolution		= 200;
			//terrainSettings.seed			= 1234;
			//terrainSettings.scale			= 550.0f;
			//terrainSettings.lacunarity		= 1.5f;
			//terrainSettings.octaveWeights	= { 1.0f, 0.3f, 0.15f, 0.10f, 0.10f, 0.05f };
			//
			//Entity terrain = world.CreateEntity(TerrainComponent(terrainSettings));

			CameraComponent camera(windowInfo.width, windowInfo.height, 70.0f, 0.001f, 1000.f);
			TransformComponent cameraTransform({ 0.0f, 12.0f, 4.0f }, { { 0.0f, 1.0f, 0.0f }, ToRadians(0.0f)}, {1.0f, 1.0f, 1.0f});
			gCamera = world.CreateEntity(camera, cameraTransform, cameraPhysics);

			uSize maxInFlightCount = 3;
			VulkanRenderer* pRenderer = new VulkanRenderer(gfx, *gfx.pPrimaryDevice, *gpWindow, maxInFlightCount);

			pRenderer->Initialize();
			pRenderer->Register(runtime);
			pRenderer->SetCamera(gCamera);
			pRenderer->SetTargetFPS(144);
			pRenderer->SetTargetFPS(5000);
			 
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

					float speed = superMoveSpeed ? 200.0f : (moveSpeed ? 5.0f : 0.25f);

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
						MeshComponent("Assets/Models/bunny.qmodel"), 
						testMaterial, 
						projectilePhysics);

				}
			);
		}

	}
}