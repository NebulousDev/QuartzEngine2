#pragma once

#include "../AssetHandler.h"
#include "../Assets/Shader.h"
#include "Memory/PoolAllocator.h"

namespace Quartz
{
	class QUARTZ_ENGINE_API ShaderHandler : public AssetHandler
	{
	private:
		PoolAllocator<ByteBuffer>	mBufferPool;
		PoolAllocator<Shader>		mShaderPool;

	private:
		bool LoadQShaderAsset(File& assetFile, Asset*& pOutAsset);

	public:
		ShaderHandler();

		bool LoadAsset(File& assetFile, Asset*& pOutAsset) override;
		bool UnloadAsset(Asset* pInAsset) override;
	};
}