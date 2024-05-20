#pragma once

#include "../AssetHandler.h"
#include "../Assets/Model.h"
#include "../Assets/ObjModel.h"
#include "Memory/PoolAllocator.h"

namespace Quartz
{
	class QUARTZ_ENGINE_API ModelHandler : public AssetHandler
	{
	private:
		PoolAllocator<ByteBuffer>	mBufferPool;
		PoolAllocator<Model>		mModelPool;

	private:
		bool LoadQModelAsset(File& assetFile, Asset*& pOutAsset);
		bool LoadOBJAsset(File& assetFile, Asset*& pOutAsset);

	public:
		ModelHandler();

		bool LoadAsset(File& assetFile, Asset*& pOutAsset) override;
		bool UnloadAsset(Asset* pInAsset) override;
	};
}