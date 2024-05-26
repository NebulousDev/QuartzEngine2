#pragma once

#include "Resource/Asset.h"
#include "Types/Array.h"
#include "Utility/Hash.h"
#include "Math/Math.h"

namespace Quartz
{
	struct OBJIndex
	{
		uInt32 posIdx;
		uInt32 normIdx;
		uInt32 texIdx;

		inline bool operator==(const OBJIndex& index) const
		{
			return posIdx == index.posIdx &&
				normIdx == index.normIdx &&
				texIdx == index.texIdx;
		}

		inline bool operator!=(const OBJIndex& index) const
		{
			return !operator==(index);
		}
	};

	template<>
	inline uSize Hash<OBJIndex>(const OBJIndex& value)
	{
		uInt64 intVal = 0;
		intVal = (value.posIdx & 0x001fffff);
		intVal |= (value.normIdx & 0x001fffff) >> 21;
		intVal |= value.texIdx >> 21;

		uInt64 hash = 525201411107845655ull;

		hash ^= intVal;
		hash *= 0x5bd1e9955bd1e995;
		hash ^= hash >> 47;

		return static_cast<uSize>(hash);
	}

	struct ObjObject
	{
		String			materialName;
		uSize			materialIdx;
		Array<OBJIndex>	indices;
	};

	struct ObjModel
	{
		Array<Vec3f>		positions;
		Array<Vec3f>		normals;
		Array<Vec2f>		texCoords;
		Array<ObjObject>	objects;
		uInt32				maxIndex;
	};
}