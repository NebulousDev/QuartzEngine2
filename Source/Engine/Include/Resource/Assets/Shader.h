#pragma once

#include "Resource/Asset.h"
#include "Types/Special/ByteBuffer.h"
#include "Types/Array.h"

namespace Quartz
{
	enum ShaderStage : uInt32
	{
		SHADER_STAGE_INVALID = 0,
		SHADER_STAGE_VERTEX,
		SHADER_STAGE_TESSELLATION_CONTROL,
		SHADER_STAGE_TESSELLATION_EVALUATION,
		SHADER_STAGE_GEOMETRY,
		SHADER_STAGE_FRAGMENT,
		SHADER_STAGE_COMPUTE,
		SHADER_STAGE_KERNEL,
		SHADER_STAGE_TASK,
		SHADER_STAGE_MESH,
		SHADER_STAGE_RAY_GENERATION,
		SHADER_STAGE_INTERSECTION,
		SHADER_STAGE_ANY_HIT,
		SHADER_STAGE_CLOSEST_HIT,
		SHADER_STAGE_MISS,
		SHADER_STAGE_CALLABLE
	};

	enum ShaderLang : uInt32
	{
		SHADER_LANG_INVALID = 0,
		SHADER_LANG_GLSL_TEXT,
		SHADER_LANG_GLSL_SPIRV,
		SHADER_LANG_HLSL_TEXT,
		SHADER_LANG_HLSL_DXBC,
		SHADER_LANG_HLSL_DXIL
	};

	enum ShaderParamType : uInt32
	{
		SHADER_PARAM_TYPE_INVALID = 0,	// 0 bytes
		SHADER_PARAM_TYPE_BOOL,			// 4 bytes
		SHADER_PARAM_TYPE_INT,			// 4 bytes
		SHADER_PARAM_TYPE_UINT,			// 4 bytes
		SHADER_PARAM_TYPE_FLOAT,		// 4 bytes
		SHADER_PARAM_TYPE_DOUBLE,		// 8 bytes
		SHADER_PARAM_TYPE_BVEC2,		// 8 bytes
		SHADER_PARAM_TYPE_BVEC3,		// 16 bytes
		SHADER_PARAM_TYPE_BVEC4,		// 16 bytes
		SHADER_PARAM_TYPE_IVEC2,		// 8 bytes
		SHADER_PARAM_TYPE_IVEC3,		// 16 bytes
		SHADER_PARAM_TYPE_IVEC4,		// 16 bytes
		SHADER_PARAM_TYPE_UVEC2,		// 8 bytes
		SHADER_PARAM_TYPE_UVEC3,		// 16 bytes
		SHADER_PARAM_TYPE_UVEC4,		// 16 bytes
		SHADER_PARAM_TYPE_VEC2,			// 8 bytes
		SHADER_PARAM_TYPE_VEC3,			// 16 bytes
		SHADER_PARAM_TYPE_VEC4,			// 16 bytes
		SHADER_PARAM_TYPE_DVEC2,		// 16 bytes
		SHADER_PARAM_TYPE_DVEC3,		// 32 bytes
		SHADER_PARAM_TYPE_DVEC4,		// 32 bytes
		SHADER_PARAM_TYPE_MAT2X2,		// ? bytes
		SHADER_PARAM_TYPE_MAT2X3,		// ? bytes
		SHADER_PARAM_TYPE_MAT2X4,		// ? bytes
		SHADER_PARAM_TYPE_MAT3X2,		// ? bytes
		SHADER_PARAM_TYPE_MAT3X3,		// ? bytes
		SHADER_PARAM_TYPE_MAT3X4,		// ? bytes
		SHADER_PARAM_TYPE_MAT4X2,		// ? bytes
		SHADER_PARAM_TYPE_MAT4X3,		// ? bytes
		SHADER_PARAM_TYPE_MAT4X4,		// 256 bytes
		SHADER_PARAM_TYPE_MAT2,			// ? bytes
		SHADER_PARAM_TYPE_MAT3,			// ? bytes
		SHADER_PARAM_TYPE_MAT4,			// 256 bytes
		SHADER_PARAM_TYPE_DMAT2X2,		// ? bytes
		SHADER_PARAM_TYPE_DMAT2X3,		// ? bytes
		SHADER_PARAM_TYPE_DMAT2X4,		// ? bytes
		SHADER_PARAM_TYPE_DMAT3X2,		// ? bytes
		SHADER_PARAM_TYPE_DMAT3X3,		// ? bytes
		SHADER_PARAM_TYPE_DMAT3X4,		// ? bytes
		SHADER_PARAM_TYPE_DMAT4X2,		// ? bytes
		SHADER_PARAM_TYPE_DMAT4X3,		// ? bytes
		SHADER_PARAM_TYPE_DMAT4X4,		// 512 bytes
		SHADER_PARAM_TYPE_DMAT2,		// ? bytes
		SHADER_PARAM_TYPE_DMAT3,		// ? bytes
		SHADER_PARAM_TYPE_DMAT4,		// 512 bytes
		SHADER_PARAM_TYPE_SAMPLER,		// ? bytes (binding)
		SHADER_PARAM_TYPE_IMAGE,		// ? bytes (binding)
		SHADER_PARAM_TYPE_ATOMIC		// 4 bytes (binding)
	};

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
	};
}