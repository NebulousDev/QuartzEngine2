#pragma once

#include "EngineAPI.h"
#include "Primitives/GraphicsShader.h"

namespace Quartz
{
	class QUARTZ_ENGINE_API ShaderCache
	{
		GraphicsShader* FindOrCreateShader(const WrapperString& shaderPath);
	};
}