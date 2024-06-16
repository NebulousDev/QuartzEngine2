#include "Resource/Loaders/ShaderHandler.h"

#include "Log.h"
#include "Resource/Binary/QShaderParser.h"

namespace Quartz
{
	bool ShaderHandler::LoadQShaderAsset(File& assetFile, Asset*& pOutAsset)
	{
		if (!assetFile.IsValid())
		{
			LogError("Error loading QShader file [%s]. Invalid file.", assetFile.GetPath().Str());
			return false;
		}

		QShaderParser qShaderParser(assetFile, &mShaderPool, &mBufferPool);

		if (!qShaderParser.Read())
		{
			LogError("Error loading QShader file [%s]. qShaderParser.Read() failed.", assetFile.GetPath().Str());
			return false;
		}

		pOutAsset = static_cast<Asset*>(qShaderParser.GetShader());

		return true;
	}

	ShaderHandler::ShaderHandler() :
		mBufferPool(2048 * sizeof(ByteBuffer)),
		mShaderPool(1024 * sizeof(Shader)) { }

	bool ShaderHandler::LoadAsset(File& assetFile, Asset*& pOutAsset)
	{
		String fileExt = assetFile.GetExtention();

		if (fileExt == "qsvert"_WRAP ||
			fileExt == "qsfrag"_WRAP || 
			fileExt == "qscomp"_WRAP || 
			fileExt == "qsgeom"_WRAP || 
			fileExt == "qstesc"_WRAP || 
			fileExt == "qstese"_WRAP || 
			fileExt == "qstask"_WRAP ||
			fileExt == "qsmesh"_WRAP || 
			fileExt == "qsrgen"_WRAP || 
			fileExt == "qsintr"_WRAP || 
			fileExt == "qsanyh"_WRAP || 
			fileExt == "qsclose"_WRAP ||
			fileExt == "qsmiss"_WRAP ||
			fileExt == "qscall"_WRAP)
		{
			return LoadQShaderAsset(assetFile, pOutAsset);
		}
		else
		{
			LogError("Failed to load shader file [%s]: unknown extension '%s'.", assetFile.GetPath().Str(), fileExt.Str());
			return false;
		}

		return false;
	}

	bool ShaderHandler::UnloadAsset(Asset* pInAsset)
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