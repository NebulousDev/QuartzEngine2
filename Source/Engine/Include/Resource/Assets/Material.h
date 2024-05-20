#pragma once

#include "Resource/Asset.h"
#include "Types/Map.h"
#include "Math/Math.h"

namespace Quartz
{
#define MATERIAL_FLOAT_SIZE				4
#define MATERIAL_INT_SIZE				4
#define MATERIAL_UINT_SIZE				4
#define MATERIAL_TEXTURE_SIZE			4
#define MATERIAL_SAMPLER_SIZE			4
#define MATERIAL_TEXTURE_SAMPLER_SIZE	4

	enum MaterialParam : uInt32
	{
		MATERIAL_PARAM_INVALID			= 0,
		MATERIAL_PARAM_FLOAT			= (1 << 16) | MATERIAL_FLOAT_SIZE * 1,
		MATERIAL_PARAM_FLOAT2			= (2 << 16) | MATERIAL_FLOAT_SIZE * 2,
		MATERIAL_PARAM_FLOAT3			= (3 << 16) | MATERIAL_FLOAT_SIZE * 3,
		MATERIAL_PARAM_FLOAT4			= (4 << 16) | MATERIAL_FLOAT_SIZE * 4,
		MATERIAL_PARAM_INT				= (5 << 16) | MATERIAL_INT_SIZE * 1,
		MATERIAL_PARAM_INT2				= (6 << 16) | MATERIAL_INT_SIZE * 2,
		MATERIAL_PARAM_INT3				= (7 << 16) | MATERIAL_INT_SIZE * 3,
		MATERIAL_PARAM_INT4				= (8 << 16) | MATERIAL_INT_SIZE * 4,
		MATERIAL_PARAM_UINT				= (9 << 16) | MATERIAL_UINT_SIZE * 1,
		MATERIAL_PARAM_UINT2			= (10 << 16) | MATERIAL_UINT_SIZE * 2,
		MATERIAL_PARAM_UINT3			= (11 << 16) | MATERIAL_UINT_SIZE * 3,
		MATERIAL_PARAM_UINT4			= (12 << 16) | MATERIAL_UINT_SIZE * 4,
		MATERIAL_PARAM_TEXTURE			= (13 << 16) | MATERIAL_TEXTURE_SIZE,
		MATERIAL_PARAM_SAMPLER			= (14 << 16) | MATERIAL_SAMPLER_SIZE,
		MATERIAL_PARAM_TEXTURE_SAMPLER	= (15 << 16) | MATERIAL_TEXTURE_SAMPLER_SIZE
	};

	struct MaterialValue
	{
		MaterialParam parameter;

		union
		{
			float	floatVal;
			Vec2f	vec2fVal;
			Vec3f	vec3fVal;
			Vec4f	vec4fVal;
			int32	intVal;
			Vec2i32 int2iVal;
			Vec3i32 int3iVal;
			Vec4i32 int4iVal;
			uInt32	uIntVal;
			Vec2u32 vec2uVal;
			Vec3u32 vec3uVal;
			Vec4u32 vec4uVal;
		};

		inline MaterialValue() : vec4fVal() {};
	};

	struct MaterialShader
	{
		String						name;
		Array<String, 8>			shaders;
	};

	struct Material : public Asset
	{
		MaterialShader				shader;
		Map<String, MaterialValue>	params;
	};
}