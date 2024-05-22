#include "Resource/Loaders/NativeShaderHandler.h"

#include "GLSLHelper.h"
#include "Memory/Memory.h"

namespace Quartz
{
	bool NativeShaderHandler::LoadGLSLShaderAsset(File& assetFile, Asset*& pOutAsset, ShaderStage stageGuess)
	{
		if (!assetFile.IsValid())
		{
			LogError("Error loading GLSL file [%s]. Invalid file.", assetFile.GetPath().Str());
			return false;
		}

		if (!assetFile.IsOpen())
		{
			if (!assetFile.Open(FILE_OPEN_READ))
			{
				LogError("Error loading config file [%s]. Open() failed.", assetFile.GetPath().Str());
				return false;
			}
		}
		else
		{
			LogWarning("Config file [%s] was already open.", assetFile.GetPath().Str());
		}

		String glslText(assetFile.GetSize());
		assetFile.Read((uInt8*)glslText.Data(), assetFile.GetSize());

		ShaderCode code;
		Array<ShaderParam> params;

		if (!ParseGLSLParams(glslText, params))
		{
			LogError("Error loading GLSL Shader: ParseGLSLParams() failed.");
			return false;
		}

		Array<uInt8> spirvData;
		if (!CompileGLSL(assetFile.GetPath(), glslText, spirvData, stageGuess))
		{
			LogError("Error loading GLSL Shader: CompileGLSL() failed.");
			return false;
		}

		code.pSourceBuffer = mBufferPool.Allocate(spirvData.Size());
		code.pSourceBuffer->Allocate(spirvData.Size());

		MemCopy(code.pSourceBuffer->Data(), spirvData.Data(), spirvData.Size());

		code.entry = "main";
		code.lang = SHADER_LANG_GLSL_SPIRV;

		Shader* pShader = mShaderPool.Allocate();
		pShader->name	= assetFile.GetPath();
		pShader->params = params;
		pShader->stage	= stageGuess;

		pShader->shaderCodes.PushBack(code);

		pOutAsset = static_cast<Asset*>(pShader);

		return true;
	}

	bool NativeShaderHandler::LoadHLSLShaderAsset(File& assetFile, Asset*& pOutAsset)
	{
		return false;
	}

	NativeShaderHandler::NativeShaderHandler() :
		mBufferPool(2048),
		mShaderPool(1024) { }

	bool NativeShaderHandler::LoadAsset(File& assetFile, Asset*& pOutAsset)
	{
		String modelExt = assetFile.GetExtention();

		if (modelExt == "vert"_STR)
		{
			return LoadGLSLShaderAsset(assetFile, pOutAsset, SHADER_STAGE_VERTEX);
		}
		else if (modelExt == "tesc"_STR)
		{
			return LoadGLSLShaderAsset(assetFile, pOutAsset, SHADER_STAGE_TESSELLATION_CONTROL);
		}
		else if (modelExt == "tese"_STR)
		{
			return LoadGLSLShaderAsset(assetFile, pOutAsset, SHADER_STAGE_TESSELLATION_EVALUATION);
		}
		else if (modelExt == "geom"_STR)
		{
			return LoadGLSLShaderAsset(assetFile, pOutAsset, SHADER_STAGE_GEOMETRY);
		}
		else if (modelExt == "frag"_STR)
		{
			return LoadGLSLShaderAsset(assetFile, pOutAsset, SHADER_STAGE_FRAGMENT);
		}
		else if (modelExt == "comp"_STR)
		{
			return LoadGLSLShaderAsset(assetFile, pOutAsset, SHADER_STAGE_COMPUTE);
		}
		else if (modelExt == "task"_STR)
		{
			return LoadGLSLShaderAsset(assetFile, pOutAsset, SHADER_STAGE_TASK);
		}
		else if (modelExt == "mesh"_STR)
		{
			return LoadGLSLShaderAsset(assetFile, pOutAsset, SHADER_STAGE_MESH);
		}
		else if (modelExt == "rgen"_STR)
		{
			return LoadGLSLShaderAsset(assetFile, pOutAsset, SHADER_STAGE_RAY_GENERATION);
		}
		else if (modelExt == "intr"_STR)
		{
			return LoadGLSLShaderAsset(assetFile, pOutAsset, SHADER_STAGE_INTERSECTION);
		}
		else if (modelExt == "anyh"_STR)
		{
			return LoadGLSLShaderAsset(assetFile, pOutAsset, SHADER_STAGE_ANY_HIT);
		}
		else if (modelExt == "close"_STR)
		{
			return LoadGLSLShaderAsset(assetFile, pOutAsset, SHADER_STAGE_CLOSEST_HIT);
		}
		else if (modelExt == "miss"_STR)
		{
			return LoadGLSLShaderAsset(assetFile, pOutAsset, SHADER_STAGE_MISS);
		}
		else if (modelExt == "call"_STR)
		{
			return LoadGLSLShaderAsset(assetFile, pOutAsset, SHADER_STAGE_CALLABLE);
		}

		else if (modelExt == "hlsl"_STR)
		{
			return LoadHLSLShaderAsset(assetFile, pOutAsset);
		}

		return false;
	}

	bool NativeShaderHandler::UnloadAsset(Asset* pInAsset)
	{
		return false;
	}
}