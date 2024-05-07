#pragma once

#include "../AssetLoader.h"

#include "Config/Config.h"
#include "Memory/PoolAllocator.h"

namespace Quartz
{
	class QUARTZ_ENGINE_API ConfigLoader : public AssetLoader
	{
	private:
		PoolAllocator<Config> mConfigPool;

	public:
		ConfigLoader();

		bool LoadAsset(File& assetFile, Asset*& pOutAsset) override;
		bool UnloadAsset(Asset* pInAsset) override;
	};
}