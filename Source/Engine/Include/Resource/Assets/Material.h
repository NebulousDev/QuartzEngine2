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

	enum MaterialValueType : uInt32
	{
		MATERIAL_VALUE_INVALID			= 0,
		MATERIAL_VALUE_FLOAT			= (1 << 16) | MATERIAL_FLOAT_SIZE * 1,
		MATERIAL_VALUE_VEC2				= (2 << 16) | MATERIAL_FLOAT_SIZE * 2,
		MATERIAL_VALUE_VEC3				= (3 << 16) | MATERIAL_FLOAT_SIZE * 3,
		MATERIAL_VALUE_VEC4				= (4 << 16) | MATERIAL_FLOAT_SIZE * 4,
		MATERIAL_VALUE_INT				= (5 << 16) | MATERIAL_INT_SIZE * 1,
		MATERIAL_VALUE_IVEC2			= (6 << 16) | MATERIAL_INT_SIZE * 2,
		MATERIAL_VALUE_IVEC3			= (7 << 16) | MATERIAL_INT_SIZE * 3,
		MATERIAL_VALUE_IVEC4			= (8 << 16) | MATERIAL_INT_SIZE * 4,
		MATERIAL_VALUE_UINT				= (9 << 16) | MATERIAL_UINT_SIZE * 1,
		MATERIAL_VALUE_UVEC2			= (10 << 16) | MATERIAL_UINT_SIZE * 2,
		MATERIAL_VALUE_UVEC3			= (11 << 16) | MATERIAL_UINT_SIZE * 3,
		MATERIAL_VALUE_UVEC4			= (12 << 16) | MATERIAL_UINT_SIZE * 4,
		MATERIAL_VALUE_TEXTURE			= (13 << 16) | MATERIAL_TEXTURE_SIZE,
	};

	struct MaterialValue
	{
		MaterialValueType type;

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
			char    _data[16] = {};
		};
		
		String	stringVal;

		inline MaterialValue() : _data{} {};
		inline MaterialValue(const MaterialValue& value) : 
			type(value.type), vec4uVal(value.vec4uVal), stringVal(value.stringVal) {}

		inline MaterialValue& operator=(const MaterialValue& value)
		{
			type = value.type;
			vec4uVal = value.vec4uVal;
			stringVal = value.stringVal;
			return *this;
		}

		inline friend bool operator==(const MaterialValue& val0, const MaterialValue& val1)
		{
			return val0.type == val1.type;
		}

		//inline ~MaterialValue()
		//{
		//	if (type == MATERIAL_VALUE_TEXTURE)
		//	{
		//		stringVal.~StringBase();
		//	}
		//}
	};

	struct Material : public Asset
	{
		Array<String, 8>			shaderPaths;
		Map<String, MaterialValue>	shaderValues;

		inline Material() = default;
		inline Material(File* pSourceFile) : Asset(pSourceFile) {};

		inline String GetAssetTypeName() const override { return "Material"; }
	};
}