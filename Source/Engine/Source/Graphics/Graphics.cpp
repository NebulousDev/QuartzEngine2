#include "Graphics/Graphics.h"

#include "Log.h"
#include "Engine.h"

namespace Quartz
{
	Graphics::Graphics() :
		mpActiveApi(nullptr) {}

	Graphics::~Graphics()
	{
		delete mpFrameGraph;
	}

	void Graphics::ApiStart()
	{
		mpActiveApi->apiFunctions.apiStartFunc();
	}

	void Graphics::ApiStop()
	{
		mpActiveApi->apiFunctions.apiStopFunc();
	}

	void Graphics::ApiWaitIdle()
	{
		mpActiveApi->apiFunctions.apiWaitIdleFunc();
	}

	void Graphics::Update(Runtime& runtime, double deltaTime)
	{
		//ApiWaitIdle();

		mpFrameGraph->Reset();

		for (Renderer* pRenderer : mRenderers)
		{
			pRenderer->OnUpdate(deltaTime);
		}
		
		for (Renderer* pRenderer : mRenderers)
		{
			pRenderer->OnBuildFrame(*mpFrameGraph);
		}

		mpFrameGraph->Build();
		mpFrameGraph->Execute();
	}

	bool Graphics::Start(const String& apiName)
	{
		auto& rendererIt = mApis.Find(apiName);
		if (rendererIt != mApis.End())
		{
			GraphicsApiInfo& info = rendererIt->value;
			bool result = info.apiFunctions.apiStartFunc();

			if (!result)
			{
				LogFatal("Error starting Graphics Api [name=\"%s\"]: Api Start() returned 'false'. Check error logs.");
				return false;
			}

			mpActiveApi = &info;
			mpFrameGraph = info.pFrameGraph;

			mpFrameGraph->ApiInitialize(info);

			Engine::GetRuntime().RegisterOnUpdate(&Graphics::Update, this);

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

			Engine::GetRuntime().UnregisterOnUpdate(&Graphics::Update, this);

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
		pRenderer->OnDestroy();
		mRenderers.RemoveIndex(index);
		mRendererMap.Remove(rendererIt);

		return true;
	}

	void Graphics::RegisterApi(const String& apiName, const GraphicsApiInfo& apiInfo)
	{
		mApis.Put(apiName, apiInfo);
	}

	bool Graphics::SetMultipleBuffering(uSize count)
	{
		if (count > 3)
		{
			LogError("Error setting multiple buffering: Count of %d is greater than 3.", count);
			return false;
		}
		else if (count < 1)
		{
			LogError("Error setting multiple buffering: Count of %d is less than 1.", count);
			return false;
		}

		mMultipleBufferCount = count;
		mFlagRebuildGraphs = true;

		return true;
	}

	ShaderCache& Graphics::GetShaderCache()
	{
		return mShaderCache;
	}

	PipelineCache& Graphics::GetPipelineCache()
	{
		return mPipelineCache;
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

	const GraphicsApiInfo* Graphics::GetCurrentApiInfo() const
	{
		return mpActiveApi;
	}

	const GraphicsApiInfo* Graphics::GetApiInfo(const String& apiName) const
	{
		auto& apiIt = mApis.Find(apiName);
		if (apiIt != mApis.End())
		{
			return &apiIt->value;
		}

		return nullptr;
	}
}