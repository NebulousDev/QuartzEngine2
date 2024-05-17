#pragma once

#include "Types/Types.h"
#include "Memory/Memory.h"

namespace Quartz
{
	enum QMFShaderType : uInt16
	{
		QMF_SHADER_TYPE_VERTEX,
		QMF_SHADER_TYPE_TESSELLATION_CONTROL,
		QMF_SHADER_TYPE_TESSELLATION_EVALUATION,
		QMF_SHADER_TYPE_GEOMETRY,
		QMF_SHADER_TYPE_FRAGMENT,
		QMF_SHADER_TYPE_COMPUTE,
		QMF_SHADER_TYPE_KERNEL,
		QMF_SHADER_TYPE_TASK,
		QMF_SHADER_TYPE_MESH,
		QMF_SHADER_TYPE_RAY_GENERATION,
		QMF_SHADER_TYPE_INTERSECTION,
		QMF_SHADER_TYPE_ANY_HIT,
		QMF_SHADER_TYPE_CLOSEST_HIT,
		QMF_SHADER_TYPE_MISS,
		QMF_SHADER_TYPE_CALLABLE
	};

	enum QMFShaderLang : uInt16
	{
		QMF_SHADER_LANG_GLSL_TEXT,
		QMF_SHADER_LANG_GLSL_SPIRV,
		QMF_SHADER_LANG_HLSL_TEXT,
		QMF_SHADER_LANG_HLSL_CSO
	};

	enum QMFStringEncoding : uInt16
	{
		QMF_STRING_UTF8 = 1,
		QMF_STRING_UTF16 = 2,
		QMF_STRING_UTF32 = 4
	};

	using QMFStringID = uInt32;

	enum QMFVertexAttribute : uInt8
	{
		QMF_VERTEX_ATTRIBUTE_INVALID = 0,
		QMF_VERTEX_ATTRIBUTE_POSITION,
		QMF_VERTEX_ATTRIBUTE_NORMAL,
		QMF_VERTEX_ATTRIBUTE_BINORMAL,
		QMF_VERTEX_ATTRIBUTE_TANGENT,
		QMF_VERTEX_ATTRIBUTE_TEXCOORD,
		QMF_VERTEX_ATTRIBUTE_COLOR,
		QMF_VERTEX_ATTRIBUTE_DATA
	};

	enum QMFVertexFormat : uInt8
	{
		QMF_VERTEX_FORMAT_INVALID = 0,
		QMF_VERTEX_FORMAT_FLOAT,
		QMF_VERTEX_FORMAT_FLOAT2,
		QMF_VERTEX_FORMAT_FLOAT3,
		QMF_VERTEX_FORMAT_FLOAT4,
		QMF_VERTEX_FORMAT_INT,
		QMF_VERTEX_FORMAT_INT2,
		QMF_VERTEX_FORMAT_INT3,
		QMF_VERTEX_FORMAT_INT4,
		QMF_VERTEX_FORMAT_UINT,
		QMF_VERTEX_FORMAT_UINT2,
		QMF_VERTEX_FORMAT_UINT3,
		QMF_VERTEX_FORMAT_UINT4,
		QMF_VERTEX_FORMAT_INT_2_10_10_10,
		QMF_VERTEX_FORMAT_UINT_2_10_10_10,
		QMF_VERTEX_FORMAT_FLOAT_10_11_11
	};

	enum QMFIndexFormat : uInt8
	{
		QMF_INDEX_FORMAT_INVALID = 0,
		QMF_INDEX_FORMAT_UINT8,
		QMF_INDEX_FORMAT_UINT16,
		QMF_INDEX_FORMAT_UINT32
	};

#pragma pack(push,1)

	struct QMFStringTable							// 256 bits
	{
		char				magic[8] = "QMFSTRS";	// 64 bits
		uInt32				stringCount;			// 32 bits
		QMFStringEncoding	encoding;				// 16 bits
		uInt16				_reserved0;				// 16 bits
		uInt64				strsOffset;				// 64 bits
		uInt64				strsSizeBytes;			// 64 bits
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

	struct QMFShaderTable							// 320 bits
	{
		char			magic[8] = "QMFSDRS";		// 64 bits
		uInt32			shaderCount;				// 32 bits
		uInt32			_reserved0;					// 32 bits
		uInt64			shadersOffset;				// 64 bits
		uInt64			shadersSizeBytes;			// 64 bits
		uInt64			nextExtOffset;				// 64 bits
	};

	struct QMFMeshElement							// 32 bits
	{
		QMFVertexAttribute	attribute;				// 8 bits
		QMFVertexFormat		format;					// 8 bits
		uInt16				_reserved0;				// 16 bits
	};

	struct QMFMesh									// 448 bits
	{
		QMFStringID		meshNameID;					// 32 bits
		QMFIndexFormat	indexFormat;				// 8 bits
		uInt8			_reserved0;					// 8 bits
		uInt32			_reserved1;					// 16 bits
		uInt16			elementCount;				// 16 bits
		uInt16			elementOffset;				// 16 bits
		uInt32			_reserved2;					// 32 bits
		uInt64			verticesOffset;				// 64 bits
		uInt64			verticesSizeBytes;			// 64 bits
		uInt64			indicesOffset;				// 64 bits
		uInt64			indicesSizeBytes;			// 64 bits
		uInt64			nextExtOffset;				// 64 bits
	};

	struct QMFMeshTable								// 320 bits
	{
		char	magic[8] = "QMFMSHS";				// 64 bits
		uInt32	meshCount;							// 32 bits
		uInt16	_reserved0;							// 32 bits
		uInt64	meshesOffset;						// 64 bits
		uInt64	meshesSizeBytes;					// 64 bits
		uInt64	nextExtOffset;						// 64 bits
	};

	struct QMFLayout								// 64 bits
	{
		uInt16 shaderCount;							// 16 bits
		uInt16 materialCount;						// 16 bits
		uInt16 meshCount;							// 16 bits
		uInt16 _reserved0;							// 16 bits
	};

	struct QMF
	{
		char			magic[8] = "QMF";			// 32 bits
		uInt16			version;					// 16 bits
		uInt16			_reserved0;					// 16 bits
		uInt32			_reserved1;					// 32 bits
		QMFLayout		layout;						// 64 bits
		QMFStringTable	stringTable;				// 256 bits
		QMFMeshTable	meshTable;					// 320 bits
		uInt64			nextExtOffset;				// 64 bits
	};

#pragma pack(pop)

}