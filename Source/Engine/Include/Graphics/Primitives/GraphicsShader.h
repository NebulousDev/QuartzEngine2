#pragma once

#include "GraphicsObject.h"
#include "Resource/Common.h"
#include "Types/String.h"
#include "Types/Array.h"

namespace Quartz
{
	class QUARTZ_ENGINE_API GraphicsShader : public GraphicsObject
	{
	private:
		String					mName;
		ShaderStage				mStage;
		Array<ShaderUniform>	mUniforms;

	public:

	};
}