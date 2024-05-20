#pragma once

#include "QCommon.h"

namespace Quartz
{
	enum QMFShaderType : uInt16
	{
		QMODEL_SHADER_TYPE_VERTEX,
		QMODEL_SHADER_TYPE_TESSELLATION_CONTROL,
		QMODEL_SHADER_TYPE_TESSELLATION_EVALUATION,
		QMODEL_SHADER_TYPE_GEOMETRY,
		QMODEL_SHADER_TYPE_FRAGMENT,
		QMODEL_SHADER_TYPE_COMPUTE,
		QMODEL_SHADER_TYPE_KERNEL,
		QMODEL_SHADER_TYPE_TASK,
		QMODEL_SHADER_TYPE_MESH,
		QMODEL_SHADER_TYPE_RAY_GENERATION,
		QMODEL_SHADER_TYPE_INTERSECTION,
		QMODEL_SHADER_TYPE_ANY_HIT,
		QMODEL_SHADER_TYPE_CLOSEST_HIT,
		QMODEL_SHADER_TYPE_MISS,
		QMODEL_SHADER_TYPE_CALLABLE
	};

	enum QMFShaderLang : uInt16
	{
		QMODEL_SHADER_LANG_GLSL_TEXT,
		QMODEL_SHADER_LANG_GLSL_SPIRV,
		QMODEL_SHADER_LANG_HLSL_TEXT,
		QMODEL_SHADER_LANG_HLSL_CSO
	};

	struct QMFShader								// 384 bits
	{
		QMFStringID		shaderNameID;				// 32 bits
		QMFStringID		shaderEntryID;				// 32 bits
		QMFShaderType	shaderType;					// 16 bits
		QMFShaderLang	shaderLang;					// 16 bits
		uInt32			_reserved1;					// 32 bits

		uInt64			paramsOffset;				// 64 bits
		uInt64			paramsSizeBytes;			// 64 bits
		uInt64			codeOffset;					// 64 bits
		uInt64			codeSizeBytes;				// 64 bits
	};

	struct QMFShaderTable							// 256 bits
	{
		uInt32			shaderCount;				// 32 bits
		uInt32			_reserved0;					// 32 bits
		uInt64			shadersOffset;				// 64 bits
		uInt64			shadersSizeBytes;			// 64 bits
		uInt64			nextExtOffset;				// 64 bits
	};
}