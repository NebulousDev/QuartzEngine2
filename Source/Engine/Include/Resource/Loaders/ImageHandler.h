#pragma once

#include "Resource/AssetHandler.h"
#include "Resource/Assets/Image.h"
#include "Memory/PoolAllocator.h"

namespace Quartz
{
	class QUARTZ_ENGINE_API ImageHandler : public AssetHandler
	{
	private:
		PoolAllocator<ByteBuffer>	mBufferPool;
		PoolAllocator<Image>		mImagePool;

	public:
		ImageHandler();

		bool LoadAsset(File& assetFile, Asset*& pOutAsset) override;
		bool UnloadAsset(Asset* pInAsset) override;
	};
}