#pragma once

#include "Graphics/Renderer.h"

namespace Quartz
{
	class QUARTZ_ENGINE_API WorldRenderer : public Renderer
	{
	private:

	public:
		void OnInitialize() override;
		void OnDestroy() override;
		void OnUpdate(double deltaTime) override;
		void OnBuildFrame(FrameGraph& frameGraph) override;
	};
}