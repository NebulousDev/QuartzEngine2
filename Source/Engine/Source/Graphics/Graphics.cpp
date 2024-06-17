#include "Graphics/Graphics.h"

#include "Log.h"
#include "Engine.h"

#define QUARTZ_GRAPHICS_IDLE_TIMEOUT 10000000

namespace Quartz
{
	Graphics::Graphics() :
		mpActiveApi(nullptr) {}

	Graphics::~Graphics()
	{
		delete mpFrameGraph;
	}

	bool Graphics::ApiStart()
	{
		return mpActiveApi->apiFunctions.apiStart();
	}

	bool Graphics::ApiStop()
	{
		return mpActiveApi->apiFunctions.apiStop();
	}

	void Graphics::ApiWaitIdle(uInt64 timeout)
	{
		mpActiveApi->apiFunctions.apiWaitIdle(timeout);
	}

	bool Graphics::ApiCreateImage(const GraphicsImageInfo& imageInfo, GraphicsMemoryInfo& outMemoryInfo, void*& pOutNativeImage)
	{
		bool result = mpActiveApi->apiFunctions.apiCreateImage(imageInfo, outMemoryInfo, pOutNativeImage);
		return result;
	}

	bool Graphics::ApiDestroyImage(void* pNativeImage)
	{
		bool result = mpActiveApi->apiFunctions.apiDestroyImage(pNativeImage);
		return result;
	}

	void Graphics::Update(Runtime& runtime, double deltaTime)
	{
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
		auto& rendererIt = mAvailableApis.Find(apiName);
		if (rendererIt != mAvailableApis.End())
		{
			GraphicsApiInfo& info = rendererIt->value;
			bool result = ApiStart();

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

			ApiWaitIdle(QUARTZ_GRAPHICS_IDLE_TIMEOUT);

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
			ApiWaitIdle(QUARTZ_GRAPHICS_IDLE_TIMEOUT);

			bool result = ApiStop();

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
		if (mAvailableApis.Size() == mAvailableApis.Capacity())
		{
			LogFatal("Error registering Graphics API [name=%s]: Too many APIs registered.", apiName.Str());
			return;
		}
		else if (mAvailableApis.Contains(apiName))
		{
			LogWarning("Warning registering Graphics API [name=%s]: An API by that name is already registered. Overwriting API...", apiName.Str());
		}

		mAvailableApis.Put(apiName, apiInfo);
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
		Array<String> apiNames(mAvailableApis.Size());

		uSize idx = 0;
		for (auto& apiPair : mAvailableApis)
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
		auto& apiIt = mAvailableApis.Find(apiName);
		if (apiIt != mAvailableApis.End())
		{
			return &apiIt->value;
		}

		return nullptr;
	}
}