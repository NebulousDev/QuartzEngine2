#pragma once

#include "Types/String.h"
#include "Types/Array.h"
#include <functional>

namespace Quartz
{
	class FrameGraph;

	using GraphicsApiStartFunc		= std::function<bool()>;
	using GraphicsApiStopFunc		= std::function<bool()>;
	using GraphicsApiWaitIdleFunc	= std::function<bool()>;

	struct GraphicsApiFunctions
	{
		GraphicsApiStartFunc	apiStartFunc;
		GraphicsApiStopFunc		apiStopFunc;
		GraphicsApiWaitIdleFunc	apiWaitIdleFunc;
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