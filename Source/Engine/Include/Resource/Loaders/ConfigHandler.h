#pragma once

#include "../AssetHandler.h"
#include "Config/Config.h"
#include "Memory/PoolAllocator.h"

namespace Quartz
{
	class QUARTZ_ENGINE_API ConfigHandler : public AssetHandler
	{
	private:
		PoolAllocator<Config> mConfigPool;

	public:
		ConfigHandler();

		bool LoadAsset(File& assetFile, Asset*& pOutAsset) override;
		bool UnloadAsset(Asset* pInAsset) override;
	};
}