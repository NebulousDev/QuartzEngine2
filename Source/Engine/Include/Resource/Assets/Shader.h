#pragma once

#include "Resource/Asset.h"
#include "Resource/Common.h"
#include "Types/Special/ByteBuffer.h"
#include "Types/Array.h"

namespace Quartz
{
	struct ShaderParam
	{
		String			name;
		ShaderParamType	type;
		uInt32			set;
		uInt32			binding;
		uInt32			arrayCount;
		uInt32			valueOffsetBytes;
		uInt32			valueSizeBytes;
	};

	struct ShaderCode
	{
		ByteBuffer* pSourceBuffer;
		ShaderLang	lang;
		String		entry;
	};

	struct Shader : public Asset
	{
		String					name;
		ShaderStage				stage;
		Array<ShaderCode, 5>	shaderCodes;
		Array<ShaderParam>		params;

		Shader() = default;
		Shader(File* pSourceFile) : Asset(pSourceFile) {};

		inline String GetAssetTypeName() const override { return "Shader"; }
	};
}