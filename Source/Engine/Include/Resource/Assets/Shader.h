#pragma once

#include "Resource/Asset.h"
#include "Resource/Common.h"
#include "Types/Special/ByteBuffer.h"
#include "Types/Array.h"

namespace Quartz
{
	struct Shader : public Asset
	{
		struct ShaderCode
		{
			ByteBuffer* pSourceBuffer;
			ShaderLang	lang;
			String		entry;
		};

		String					name;
		ShaderStage				stage;
		Array<ShaderCode, 5>	shaderCodes;
		Array<ShaderUniform>	uniforms;

		Shader() = default;
		Shader(File* pSourceFile) : Asset(pSourceFile) {};

		inline String GetAssetTypeName() const override { return "Shader"; }
	};
}