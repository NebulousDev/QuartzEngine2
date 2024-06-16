#include "Resource/Loaders/NativeShaderHandler.h"

#include "GLSLHelper.h"
#include "Memory/Memory.h"
#include "Resource/SPIRV/Spirv.h"

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

		Shader::ShaderCode code;
		Array<ShaderUniform> params;

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

		SpirvReflection reflection;

		if (SpirvParseReflection(&reflection, *code.pSourceBuffer))
		{
			code.entry = reflection.entryName;
		}
		else
		{
			LogWarning("Failed to parse SPIR-V reflection data. EntryName defaulting to \"main\"");
			code.entry = "main";
		}

		code.lang = SHADER_LANG_GLSL_SPIRV;

		Shader* pShader = mShaderPool.Allocate(&assetFile);

		if (!pShader)
		{
			return false;
		}

		pShader->name		= assetFile.GetPath();
		pShader->uniforms	= params;
		pShader->stage		= stageGuess;

		pShader->shaderCodes.PushBack(code);

		pOutAsset = static_cast<Asset*>(pShader);

		return true;
	}

	bool NativeShaderHandler::LoadHLSLShaderAsset(File& assetFile, Asset*& pOutAsset)
	{
		return false;
	}

	NativeShaderHandler::NativeShaderHandler() :
		mBufferPool(2048 * sizeof(ByteBuffer)),
		mShaderPool(1024 * sizeof(Shader)) { }

	bool NativeShaderHandler::LoadAsset(File& assetFile, Asset*& pOutAsset)
	{
		String fileExt = assetFile.GetExtention();

		if (fileExt == "vert"_STR)
		{
			return LoadGLSLShaderAsset(assetFile, pOutAsset, SHADER_STAGE_VERTEX);
		}
		else if (fileExt == "tesc"_STR)
		{
			return LoadGLSLShaderAsset(assetFile, pOutAsset, SHADER_STAGE_TESSELLATION_CONTROL);
		}
		else if (fileExt == "tese"_STR)
		{
			return LoadGLSLShaderAsset(assetFile, pOutAsset, SHADER_STAGE_TESSELLATION_EVALUATION);
		}
		else if (fileExt == "geom"_STR)
		{
			return LoadGLSLShaderAsset(assetFile, pOutAsset, SHADER_STAGE_GEOMETRY);
		}
		else if (fileExt == "frag"_STR)
		{
			return LoadGLSLShaderAsset(assetFile, pOutAsset, SHADER_STAGE_FRAGMENT);
		}
		else if (fileExt == "comp"_STR)
		{
			return LoadGLSLShaderAsset(assetFile, pOutAsset, SHADER_STAGE_COMPUTE);
		}
		else if (fileExt == "task"_STR)
		{
			return LoadGLSLShaderAsset(assetFile, pOutAsset, SHADER_STAGE_TASK);
		}
		else if (fileExt == "mesh"_STR)
		{
			return LoadGLSLShaderAsset(assetFile, pOutAsset, SHADER_STAGE_MESH);
		}
		else if (fileExt == "rgen"_STR)
		{
			return LoadGLSLShaderAsset(assetFile, pOutAsset, SHADER_STAGE_RAY_GENERATION);
		}
		else if (fileExt == "intr"_STR)
		{
			return LoadGLSLShaderAsset(assetFile, pOutAsset, SHADER_STAGE_INTERSECTION);
		}
		else if (fileExt == "anyh"_STR)
		{
			return LoadGLSLShaderAsset(assetFile, pOutAsset, SHADER_STAGE_ANY_HIT);
		}
		else if (fileExt == "close"_STR)
		{
			return LoadGLSLShaderAsset(assetFile, pOutAsset, SHADER_STAGE_CLOSEST_HIT);
		}
		else if (fileExt == "miss"_STR)
		{
			return LoadGLSLShaderAsset(assetFile, pOutAsset, SHADER_STAGE_MISS);
		}
		else if (fileExt == "call"_STR)
		{
			return LoadGLSLShaderAsset(assetFile, pOutAsset, SHADER_STAGE_CALLABLE);
		}

		else if (fileExt == "hlsl"_STR)
		{
			return LoadHLSLShaderAsset(assetFile, pOutAsset);
		}

		return false;
	}

	bool NativeShaderHandler::UnloadAsset(Asset* pInAsset)
	{
		Shader* pShader = static_cast<Shader*>(pInAsset);

		for (Shader::ShaderCode& code : pShader->shaderCodes)
		{
			mBufferPool.Free(code.pSourceBuffer);
		}

		mShaderPool.Free(pShader);

		return true;
	}
}