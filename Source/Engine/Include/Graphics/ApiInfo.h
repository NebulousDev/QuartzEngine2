#pragma once

#include "Types/String.h"
#include "Types/Array.h"
#include <functional>

#include "Graphics/Primitives/GraphicsImage.h"

namespace Quartz
{
	class FrameGraph;

	using GraphicsApiStartFunc		= std::function<bool()>;
	using GraphicsApiStopFunc		= std::function<bool()>;
	using GraphicsApiWaitIdleFunc	= std::function<bool(uInt64 timeout)>;

	using GraphicsApiCreateImageFunc	= std::function<bool(const GraphicsImageInfo& imageInfo, GraphicsMemoryInfo& outMemoryInfo, void*& pOutNativeImage)>;
	using GraphicsApiDestroyImageFunc	= std::function<bool(void* pNativeImage)>;

	struct GraphicsApiFunctions
	{
		GraphicsApiStartFunc		apiStart;
		GraphicsApiStopFunc			apiStop;
		GraphicsApiWaitIdleFunc		apiWaitIdle;

		GraphicsApiCreateImageFunc	apiCreateImage;
		GraphicsApiDestroyImageFunc	apiDestroyImage;
	};

	struct GraphicsApiInfo
	{
		String					fullName;
		String					version;
		Array<String>			capabilities;
		GraphicsApiFunctions	apiFunctions;
		FrameGraph*				pFrameGraph;
		void*					pNativeApi;
	};
}