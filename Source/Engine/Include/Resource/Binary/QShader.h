#pragma once

#include "QCommon.h"

namespace Quartz
{
	enum QShaderStage : uInt16
	{
		QSHADER_STAGE_INVALID = 0,
		QSHADER_STAGE_VERTEX,
		QSHADER_STAGE_TESSELLATION_CONTROL,
		QSHADER_STAGE_TESSELLATION_EVALUATION,
		QSHADER_STAGE_GEOMETRY,
		QSHADER_STAGE_FRAGMENT,
		QSHADER_STAGE_COMPUTE,
		QSHADER_STAGE_KERNEL,
		QSHADER_STAGE_TASK,
		QSHADER_STAGE_MESH,
		QSHADER_STAGE_RAY_GENERATION,
		QSHADER_STAGE_INTERSECTION,
		QSHADER_STAGE_ANY_HIT,
		QSHADER_STAGE_CLOSEST_HIT,
		QSHADER_STAGE_MISS,
		QSHADER_STAGE_CALLABLE
	};

	enum QShaderLang : uInt16
	{
		QSHADER_LANG_INVALID		= 0x00,
		QSHADER_LANG_GLSL_TEXT		= 0x01,
		QSHADER_LANG_GLSL_SPIRV		= 0x02,
		QSHADER_LANG_HLSL_TEXT		= 0x04,
		QSHADER_LANG_HLSL_DXBC		= 0x08,
		QSHADER_LANG_HLSL_DXIL		= 0x10,
	};

	using QShaderLangs = uInt16;

	enum QShaderParamType : uInt16
	{
		QSHADER_PARAM_TYPE_INVALID = 0,
		QSHADER_PARAM_TYPE_INT
	};

#pragma pack(push,1)

	struct QShaderParam								// 256 bits
	{
		QStringID			nameID;					// 32 bits
		uInt16				_reserved0;				// 16 bits
		QShaderParamType	type;				// 16 bits
		uInt32				set;					// 32 bits
		uInt32				binding;				// 32 bits
		uInt32				arrayCount;				// 32 bits
		uInt32				_reserved1;				// 32 bits
		uInt32				valueOffsetBytes;		// 32 bits
		uInt32				valueSizeBytes;			// 32 bits
	};

	struct QShaderCode								// 192 bits
	{
		QStringID			entryID;				// 32 bits
		QShaderLang			lang;					// 16 bits
		uInt16				_reserved0;				// 16 bits
		uInt64				codeOffset;				// 64 bits
		uInt64				codeSizeBytes;			// 64 bits
	};

	struct QShaderShaderTable						// 256 bits
	{
		uInt32 shaderCount;							// 32 bits
		uInt32 _reserved0;							// 32 bits
		uInt64 shadersOffset;						// 64 bits
		uInt64 shadersSizeBytes;					// 64 bits
		uInt64 nextExtOffset;						// 64 bits
	};

	struct QShaderParamTable						// 256 bits
	{
		uInt32 paramsCount;							// 32 bits
		uInt32 _reserved0;							// 32 bits
		uInt64 paramsOffset;						// 64 bits
		uInt64 paramsSizeBytes;						// 64 bits
		uInt64 nextExtOffset;						// 64 bits
	};

	struct QShader
	{
		char				magic[8] = "QShader";	// 32 bits
		uInt16				versionMajor;			// 16 bits
		uInt16				versionMinor;			// 16 bits
		uInt64				_reserved0;				// 64 bits
		QShaderStage		stage;					// 16 bits
		QShaderLangs		sourceLangs;			// 16 bits
		uInt32				_reserved1;				// 32 bits
		QStringTable		stringTable;			// 256 bits
		QShaderParamTable	paramTable;				// 256 bits
		QShaderShaderTable	shaderTable;			// 256 bits
		uInt64				nextExtOffset;			// 64 bits
	};

#pragma pack(pop)

}