#include "Graphics/Graphics.h"

#include "Log.h"

namespace Quartz
{
	Graphics::Graphics() :
		mpActiveApi(nullptr)
	{
		// FrameGraph is too large to be stored in stack memory
		// We must allocate it on the heap.
		mpFrameGraph = new FrameGraph();
	}

	Graphics::~Graphics()
	{
		delete mpFrameGraph;
	}

	bool Graphics::Start(const String& apiName)
	{
		auto& rendererIt = mApis.Find(apiName);
		if (rendererIt != mApis.End())
		{
			ApiInfo& info = rendererIt->value;
			bool result = info.apiFunctions.apiStartFunc();

			if (!result)
			{
				LogFatal("Error starting Graphics Api [name=\"%s\"]: Api Start() returned 'false'. Check error logs.");
			}

			mpActiveApi = &info;
			mpFrameGraph->SetFunctions(mpActiveApi->frameGraphFunctions);

			LogInfo("Graphics started with Api [name=\"%s\"]", apiName.Str());

			return true;
		}
		else
		{
			LogFatal("Error starting Graphics Api [name=\"%s\"]: No api registered by that name.");
			return false;
		}
	}

	bool Graphics::Stop()
	{
		if (mpActiveApi)
		{
			bool result = mpActiveApi->apiFunctions.apiStopFunc();

			if (!result)
			{
				LogError("Error stopping Graphics [Api=\"%s\"]: Api Stop() returned 'false'. Check error logs.");
				return false;
			}

			mpActiveApi = nullptr;

			return true;
		}

		return false;
	}

	bool Graphics::RemoveRenderer(const String& rendererName)
	{
		auto& rendererIt = mRendererMap.Find(rendererName);
		if (rendererIt == mRendererMap.End())
		{
			return false;
		}

		uInt32 index = rendererIt->value;
		Renderer* pRenderer = mRenderers[index];
		mRenderers.RemoveIndex(index);
		mRendererMap.Remove(rendererIt);

		return true;
	}

	void Graphics::RegisterApi(const String& apiName, const Graphics::ApiInfo& apiInfo)
	{
		mApis.Put(apiName, apiInfo);
	}

	FrameGraph& Graphics::GetFrameGraph()
	{
		return *mpFrameGraph;
	}

	const Array<String> Graphics::GetApiNames() const
	{
		Array<String> apiNames(mApis.Size());

		uSize idx = 0;
		for (auto& apiPair : mApis)
		{
			apiNames[idx++] = apiPair.key;
		}

		return apiNames;
	}

	const Graphics::ApiInfo* Graphics::GetCurrentApiInfo() const
	{
		return mpActiveApi;
	}

	const Graphics::ApiInfo* Graphics::GetApiInfo(const String& apiName) const
	{
		auto& apiIt = mApis.Find(apiName);
		if (apiIt != mApis.End())
		{
			return &apiIt->value;
		}

		return nullptr;
	}
}