#pragma once

#include "Types/String.h"
#include "Math/Math.h"

namespace Quartz
{
	struct MtlMaterial
	{
		String	name;
		Vec3f	kaAmbientColor;
		Vec3f	kdDiffuseColor;
		Vec3f	ksSpecularColor;
		float	nsSpecularExp;
		String	mapKaAmbientTexture;
		String	mapKdDiffuseTexture;
		String	mapKsSpecularTexture;
		String	mapNsSpecularExpTexture;
		String	mapPrRoughnessTexture;
		String	mapPmMetallicTexture;
		String	mapPsSheenTexture;
		String	mapPcClearcoatThickTexture;
		String	mapPcrClearcoatRoughTexture;
		String	mapKeEmissiveTexture;
		String	mapNormNormalTexture;
	};
}