#pragma once

#include "../../GfxAPI.h"
#include "Resource/AssetHandler.h"
#include "Resource/Assets/Shader.h"
#include "Memory/PoolAllocator.h"

namespace Quartz
{
	class QUARTZ_GRAPHICS_API NativeShaderHandler : public AssetHandler
	{
	private:
		PoolAllocator<ByteBuffer>	mBufferPool;
		PoolAllocator<Shader>		mShaderPool;

	private:
		bool LoadGLSLShaderAsset(File& assetFile, Asset*& pOutAsset, ShaderStage stageGuess);
		bool LoadHLSLShaderAsset(File& assetFile, Asset*& pOutAsset);

	public:
		NativeShaderHandler();

		bool LoadAsset(File& assetFile, Asset*& pOutAsset) override;
		bool UnloadAsset(Asset* pInAsset) override;
	};
}