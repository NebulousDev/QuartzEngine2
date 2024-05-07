#include "Resource/Loaders/ConfigLoader.h"

#include "Log.h"

namespace Quartz
{
	ConfigLoader::ConfigLoader() :
		mConfigPool(128) { } 

	bool ConfigLoader::LoadAsset(File& assetFile, Asset*& pOutAsset)
	{
		if (!assetFile.IsValid())
		{
			LogError("Error loading config file [%s]. Invalid file.", assetFile.GetPath().Str());
			return false;
		}

		if (!assetFile.IsOpen())
		{
			if (!assetFile.Open(FILE_OPEN_READ | FILE_OPEN_WRITE))
			{
				LogError("Error loading config file [%s]. Open() failed.", assetFile.GetPath().Str());
				return false;
			}
		}
		else
		{
			LogWarning("Config file [%s] was already open.", assetFile.GetPath().Str());
		}

		Config* pConfigAsset = mConfigPool.Allocate(&assetFile);

		if (!pConfigAsset)
		{
			LogError("Error allocating config object! [pool size=%d/%d]", mConfigPool.Size(), mConfigPool.Capacity());
			return false;
		}

		if (!pConfigAsset->Read())
		{
			LogError("Error reading config file [%s].", assetFile.GetPath().Str());
			mConfigPool.Free(pConfigAsset);
			return false;
		}

		pOutAsset = pConfigAsset;

		return true;
	}

	bool ConfigLoader::UnloadAsset(Asset* pInAsset)
	{
		if (!pInAsset)
		{
			return false;
		}

		Config* pConfigAsset = static_cast<Config*>(pInAsset);
		bool result = true;

		if (pConfigAsset->GetSourceFile()->IsOpen())
		{
			if (!pConfigAsset->GetSourceFile()->Close())
			{
				LogError("Error unloading config file [%s]. Close() failed.",
					pConfigAsset->GetSourceFile()->GetPath().Str());
				result = false;
			}
		}

		result &= mConfigPool.Free(pConfigAsset);

		return result;
	}
}