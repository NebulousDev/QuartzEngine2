#pragma once

#include "Resource/Assets/MtlMaterial.h"
#include "Resource/Assets/Material.h"

#include "Utility/StringReader.h"
#include "Types/Map.h"

namespace Quartz
{
	inline bool ParseMTL(const String& data, Array<MtlMaterial>& outMtlMaterials)
	{
		StringReader parser(data);

		Map<Substring, MtlMaterial> materials(1024); 
		MtlMaterial* pMtlMaterial = &materials.Put(("_mtl_material_"_STR).Substring(0), MtlMaterial());

		while (!parser.IsEmpty())
		{
			parser.SkipWhitespace();

			if (parser.Peek() == '\n' || parser.Peek() == '#')
			{
				parser.ReadLine();
				continue;
			}

			Substring token = parser.ReadThrough(" "_WRAP);

			// Comment
			if (token.StartsWith("#"_WRAP))
			{
				parser.ReadLine();
				continue;
			}

			else if (token == "newmtl"_WRAP)
			{
				Substring materialName = parser.ReadLine().TrimWhitespace();

				auto& materialIt = materials.Find(materialName);
				if (materialIt != materials.End())
				{
					pMtlMaterial = &materialIt->value;
				}
				else
				{
					ObjObject newObject;
					newObject.materialName = objectNameStr;
					newObject.materialIdx = nextMaterialIdx++;

					pObjObject = &objects.Put(objectNameStr, newObject);
				}
			}
		}
	}
}