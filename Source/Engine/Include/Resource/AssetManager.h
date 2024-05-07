#pragma once

#include "AssetLoader.h"
#include "Filesystem/Filesystem.h"
#include "Engine.h"
#include "Log.h"
#include "Types/Map.h"

namespace Quartz
{
	class QUARTZ_ENGINE_API AssetManager
	{
	private:
		Map<String, AssetLoader*> mLoaders;

	public:
		template<typename AssetLoaderType>
		bool RegisterAssetLoader(const String& ext, AssetLoaderType* pAssetLoader)
		{
			mLoaders.Put(ext, static_cast<AssetLoader*>(pAssetLoader));
			return true;
		}

		template<typename AssetType>
		AssetType* LoadAsset(File& assetFile)
		{
			const String ext = assetFile.GetExtention();
			AssetLoader* pLoader = nullptr;
			Asset* pAsset = nullptr;

			auto& loaderIt = mLoaders.Find(ext);
			if (loaderIt != mLoaders.End())
			{
				pLoader = loaderIt->value;
			}
			else
			{
				LogError("Error loading asset [%s]. No registered loader is associated with the extension '%s'.", 
					assetFile.GetPath().Str(), ext.Str());
				return nullptr;
			}

			if (!pLoader->LoadAsset(assetFile, pAsset))
			{
				// Error message in LoadAsset()
				return nullptr;
			}

			return static_cast<AssetType*>(pAsset);
		}

		template<typename AssetType>
		AssetType* LoadAsset(const String& path)
		{
			File* pAssetFile = Engine::GetFilesystem().GetFile(path);

			if (!pAssetFile)
			{
				LogError("Error loading asset [%s]. File does not exist.", path.Str());
				return nullptr;
			}

			return LoadAsset<AssetType>(*pAssetFile);
		}

		template<typename AssetType>
		bool UnloadAsset(AssetType* pAsset)
		{
			if (!pAsset || !pAsset->GetSourceFile())
			{
				return false;
			}

			const String ext = pAsset->GetSourceFile()->GetExtention();

			auto& loaderIt = mLoaders.Find(ext);
			if (loaderIt != mLoaders.End())
			{
				AssetLoader* pLoader = loaderIt->value;
				return pLoader->UnloadAsset(pAsset);
			}
			
			return false;
		}
	};
}