#pragma once

#include "EngineAPI.h"
#include "FrameGraph/FrameGraph.h"

namespace Quartz
{
	class QUARTZ_ENGINE_API Renderer
	{
	public:
		virtual void OnInitialize() = 0;
		virtual void OnDestroy() = 0;

		virtual void OnBackbufferChanged(uSize count, FrameGraphImageInfo& imageInfo) = 0;

		virtual void OnUpdate(double deltaTime) = 0;
		virtual void OnBuildFrame(FrameGraph& frameGraph) = 0;
	};
}