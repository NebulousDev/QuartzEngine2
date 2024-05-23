#pragma once

#include "../AssetHandler.h"
#include "../Assets/Material.h"
#include "Memory/PoolAllocator.h"

namespace Quartz
{
	class QUARTZ_ENGINE_API MaterialHandler : public AssetHandler
	{
	private:
		PoolAllocator<Material> mModelPool;

	public:
		MaterialHandler();

		bool LoadAsset(File& assetFile, Asset*& pOutAsset) override;
		bool UnloadAsset(Asset* pInAsset) override;
	};
}