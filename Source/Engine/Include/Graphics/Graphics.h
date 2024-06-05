#pragma once

#include "Types/Array.h"
#include "Types/String.h"
#include <functional>

#include "Renderer.h"
#include "FrameGraph/FrameGraph.h"

namespace Quartz
{
	class QUARTZ_ENGINE_API Graphics
	{
	public:
		using GraphicsApiStartFunc = std::function<bool()>;
		using GraphicsApiStopFunc = std::function<bool()>;

		struct ApiFunctions
		{
			GraphicsApiStartFunc	apiStartFunc;
			GraphicsApiStopFunc		apiStopFunc;
		};

		struct ApiInfo
		{
			//using namespace FrameGraph;

			String				fullName;
			String				version;
			Array<String>		capabilities;
			ApiFunctions		apiFunctions;
			FrameGraph::FrameGraphFunctions	frameGraphFunctions;
		};

	private:
		Map<String, ApiInfo>	mApis;
		ApiInfo*				mpActiveApi;
		Map<String, uInt32>		mRendererMap;
		Array<Renderer*, 64>	mRenderers;
		FrameGraph*				mpFrameGraph;

	protected:
		Graphics();
		~Graphics();

	public:
		bool Start(const String& apiName);
		bool Stop();

		template<typename RendererType>
		RendererType& AddRenderer(const String& rendererName)
		{
			Renderer* pRenderer = static_cast<Renderer*>(new RendererType());
			mRenderers.PushBack(pRenderer);
			mRendererMap.Put(rendererName, mRenderers.Size());
		}

		bool RemoveRenderer(const String& rendererName);

		void RegisterApi(const String& apiName, const ApiInfo& apiInfo);

		FrameGraph& GetFrameGraph();

		const Array<String>	GetApiNames() const;
		const ApiInfo* GetCurrentApiInfo() const;
		const ApiInfo* GetApiInfo(const String& apiName) const;
	};
}