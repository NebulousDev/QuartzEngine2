#include "Resource/Loaders/MaterialHandler.h"

#include "Log.h"
#include "Utility/StringReader.h"

namespace Quartz
{
	MaterialHandler::MaterialHandler() :
		mModelPool(1024 * sizeof(Material)) { }

	bool ParseParam(const Substring& paramValue, MaterialValue& outMaterialValue)
	{
		StringReader valueReader(paramValue);

		if (paramValue.StartsWith("texture"_WRAP))
		{
			valueReader.ReadThrough("("_WRAP);
			valueReader.SkipWhitespace();
			valueReader.ReadThrough("\""_WRAP);
			String texturePath = valueReader.ReadThrough("\""_WRAP);
			valueReader.ReadThrough(")"_WRAP);

			outMaterialValue.type		= MATERIAL_VALUE_TEXTURE;
			outMaterialValue.stringVal	= texturePath;
		}

		else if (paramValue.StartsWith("float"_WRAP))
		{
			valueReader.ReadThrough("("_WRAP);
			valueReader.SkipWhitespace();
			float val = valueReader.ReadFloat();
			valueReader.ReadThrough(")"_WRAP);

			outMaterialValue.type		= MATERIAL_VALUE_FLOAT;
			outMaterialValue.floatVal	= val;
		}

		else if (paramValue.StartsWith("vec2"_WRAP))
		{
			valueReader.ReadThrough("("_WRAP);
			valueReader.SkipWhitespace();
			float x = valueReader.ReadFloat();
			valueReader.ReadThrough(","_WRAP);
			valueReader.SkipWhitespace();
			float y = valueReader.ReadFloat();
			valueReader.ReadThrough(")"_WRAP);

			outMaterialValue.type		= MATERIAL_VALUE_VEC2;
			outMaterialValue.vec2fVal	= Vec2f(x, y);
		}

		else if (paramValue.StartsWith("vec3"_WRAP))
		{
			valueReader.ReadThrough("("_WRAP);
			valueReader.SkipWhitespace();
			float x = valueReader.ReadFloat();
			valueReader.ReadThrough(","_WRAP);
			valueReader.SkipWhitespace();
			float y = valueReader.ReadFloat();
			valueReader.ReadThrough(","_WRAP);
			valueReader.SkipWhitespace();
			float z = valueReader.ReadFloat();
			valueReader.ReadThrough(")"_WRAP);

			outMaterialValue.type		= MATERIAL_VALUE_VEC3;
			outMaterialValue.vec3fVal	= Vec3f(x, y, z);
		}

		else if (paramValue.StartsWith("vec3"_WRAP))
		{
			valueReader.ReadThrough("("_WRAP);
			valueReader.SkipWhitespace();
			float x = valueReader.ReadFloat();
			valueReader.ReadThrough(","_WRAP);
			valueReader.SkipWhitespace();
			float y = valueReader.ReadFloat();
			valueReader.ReadThrough(","_WRAP);
			valueReader.SkipWhitespace();
			float z = valueReader.ReadFloat();
			valueReader.ReadThrough(","_WRAP);
			valueReader.SkipWhitespace();
			float w = valueReader.ReadFloat();
			valueReader.ReadThrough(")"_WRAP);

			outMaterialValue.type		= MATERIAL_VALUE_VEC4;
			outMaterialValue.vec4fVal	= Vec4f(x, y, z, w);
		}
		
		return true;
	}

	bool ParseQMaterial(const String& materialPath, const String& qMaterialText, Material& outMaterial)
	{
		StringReader reader(qMaterialText);
		uSize lineNumber = 0;

		while (!reader.IsEmpty())
		{
			Substring line = reader.ReadLine();
			lineNumber++;

			if (line.StartsWith("#shader"_WRAP))
			{
				Substring shaderStr = line.Substring(7);

				const uSize shaderPathStart = shaderStr.Find("\""_WRAP);
				const uSize shaderPathEnd = shaderStr.FindReverse("\""_WRAP);

				if (shaderPathStart == shaderStr.Length() ||
					shaderPathEnd == 0)
				{
					LogError("Error parsing QMaterial file [%s] on line %d: Invalid Shader Path.", materialPath.Str(), lineNumber);
					return false;
				}

				shaderStr = shaderStr.Substring(shaderPathStart + 1, shaderPathEnd);

				outMaterial.shaderPaths.PushBack(String(shaderStr));
			}
			else if (line.StartsWith("#param"_WRAP))
			{
				Substring paramStr = line.Substring(6);

				const uSize paramNameStart = 0;
				const uSize paramNameEnd = paramStr.Find("="_WRAP);

				if (paramNameEnd == paramStr.Length())
				{
					LogError("Error parsing QMaterial file [%s] on line %d: Invalid Parameter.", materialPath.Str(), lineNumber);
					return false;
				}

				const Substring paramName = paramStr.Substring(paramNameStart, paramNameEnd).TrimWhitespace();
				const Substring paramValue = paramStr.Substring(paramNameEnd + 1).TrimWhitespace();

				MaterialValue value;
				if (!ParseParam(paramValue, value))
				{
					LogError("Error parsing QMaterial file [%s] on line %d: Invalid Parameter.", materialPath.Str(), lineNumber);
					return false;
				}

				outMaterial.shaderValues.Put(String(paramName), value);
			}
		}

		return true;
	}

	bool MaterialHandler::LoadAsset(File& assetFile, Asset*& pOutAsset)
	{
		if (!assetFile.IsValid())
		{
			LogError("Error loading QMaterial file [%s]. Invalid file.", assetFile.GetPath().Str());
			return false;
		}

		if (!assetFile.IsOpen())
		{
			if (!assetFile.Open(FILE_OPEN_READ))
			{
				LogError("Error loading QMaterial file [%s]. Open() failed.", assetFile.GetPath().Str());
				return false;
			}
		}
		else
		{
			LogWarning("QMaterial file [%s] was already open.", assetFile.GetPath().Str());
		}

		String qMaterialText(assetFile.GetSize());

		if (!assetFile.Read((uInt8*)qMaterialText.Data(), assetFile.GetSize()))
		{
			LogError("Failed to read QMaterial file [%s]!", assetFile.GetPath());
			return false;
		}
		
		Material* pMaterial = mModelPool.Allocate(&assetFile);

		if (!ParseQMaterial(assetFile.GetPath(), qMaterialText, *pMaterial))
		{
			mModelPool.Free(pMaterial);
			LogError("Error loading QMaterial file [%s]. ParseMaterial() failed.", assetFile.GetPath().Str());
			return false;
		}

		// @TODO: Make sure QMaterial has a valid combination of shaders

		pOutAsset = static_cast<Asset*>(pMaterial);

		return true;
	}

	bool MaterialHandler::UnloadAsset(Asset* pInAsset)
	{
		return true;
	}
}