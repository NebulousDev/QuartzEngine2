#pragma once

#include "Asset.h"
#include "Types/String.h"

namespace Quartz
{
	class QUARTZ_ENGINE_API AssetHandler
	{
	public:
		virtual bool LoadAsset(File& assetFile, Asset*& pOutAsset) = 0;
		virtual bool UnloadAsset(Asset* pInAsset) = 0;
	};
}