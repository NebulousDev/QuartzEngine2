#include "Graphics/Renderers/WorldRenderer.h"

#include "Engine.h"
#include "Entity/World.h"

#include "Graphics/Component/TransformComponent.h"
#include "Graphics/Component/MeshComponent.h"
#include "Graphics/Component/LightComponent.h"
#include "Graphics/Component/CameraComponent.h"

namespace Quartz
{
	void WorldRenderer::OnInitialize()
	{
		
	}

	void WorldRenderer::OnDestroy()
	{

	}

	void WorldRenderer::OnUpdate(double deltaTime)
	{

	}

	void WorldRenderer::OnBuildFrame(FrameGraph& frameGraph)
	{
		EntityWorld& world = Engine::GetWorld();

		auto& renderableView	= world.CreateView<MeshComponent, TransformComponent>();
		auto& lightView			= world.CreateView<LightComponent, TransformComponent>();

		for (Entity& entity : renderableView)
		{
			MeshComponent& meshComponent = world.Get<MeshComponent>(entity);
			TransformComponent& transformComponent = world.Get<TransformComponent>(entity);
		}

		for (Entity& entity : lightView)
		{
			MeshComponent& meshComponent = world.Get<MeshComponent>(entity);
			TransformComponent& transformComponent = world.Get<TransformComponent>(entity);
		}
	}
}