#pragma once

#include "ApiInfo.h"
#include "Renderer.h"
#include "Runtime/Runtime.h"
#include "ShaderCache.h"
#include "PipelineCache.h"
#include "ImageCache.h"
#include "FrameGraph/FrameGraph.h"

#include <functional>

namespace Quartz
{
	class QUARTZ_ENGINE_API Graphics
	{
	public:
		friend class GraphicsImageCache;
		friend class ShaderCache;
		friend class PipelineCache;

	private:
		Map<String, GraphicsApiInfo, 8>	mAvailableApis;
		GraphicsApiInfo*				mpActiveApi;
		Map<String, uInt32, 64>			mRendererMap;
		Array<Renderer*, 64>			mRenderers;
		GraphicsImageCache				mImageChache;
		ShaderCache						mShaderCache;
		PipelineCache					mPipelineCache;
		FrameGraph*						mpFrameGraph;

	protected:
		Graphics();
		~Graphics();

		bool ApiStart();
		bool ApiStop();
		void ApiWaitIdle(uInt64 timeout);

		bool ApiCreateImage(const GraphicsImageInfo& imageInfo, GraphicsMemoryInfo& outMemoryInfo, void*& pOutNativeImage);
		bool ApiDestroyImage(void* pNativeImage);

		void Update(Runtime& runtime, double deltaTime);

	public:
		Graphics(const Graphics&) = delete; // Dont accidentally copy Graphics

		bool Start(const String& apiName);
		bool Stop();

		template<typename RendererType, typename... CtorValues>
		RendererType& AddRenderer(const String& rendererName, CtorValues&&... ctorValues)
		{
			Renderer* pRenderer = static_cast<Renderer*>(new RendererType(Forward<CtorValues>(ctorValues)...));
			mRenderers.PushBack(pRenderer);
			mRendererMap.Put(rendererName, mRenderers.Size());
			pRenderer->OnInitialize();
			return *static_cast<RendererType*>(pRenderer);
		}

		bool RemoveRenderer(const String& rendererName);

		void RegisterApi(const String& apiName, const GraphicsApiInfo& apiInfo);

		ShaderCache&	GetShaderCache();
		PipelineCache&	GetPipelineCache();
		FrameGraph&		GetFrameGraph();

		const Array<String>		GetApiNames() const;
		const GraphicsApiInfo*	GetCurrentApiInfo() const;
		const GraphicsApiInfo*	GetApiInfo(const String& apiName) const;
	};
}