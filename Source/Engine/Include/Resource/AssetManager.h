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
		Map<String, AssetLoader*>	mLoaders;
		Map<String, Asset*>			mAssets;

	public:
		AssetManager() :
			mLoaders(128), mAssets(8196) {}

		template<typename AssetLoaderType>
		bool RegisterAssetLoader(const String& ext, AssetLoaderType* pAssetLoader)
		{
			mLoaders.Put(ext, static_cast<AssetLoader*>(pAssetLoader));
			return true;
		}

		template<typename AssetType>
		AssetType* GetOrLoadAsset(File& assetFile)
		{
			auto& assetIt = mAssets.Find(assetFile.GetPath());
			if (assetIt != mAssets.End())
			{
				return static_cast<AssetType*>(assetIt->value);
			}

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

			mAssets.Put(assetFile.GetPath(), pAsset);

			return static_cast<AssetType*>(pAsset);
		}

		template<typename AssetType>
		AssetType* GetOrLoadAsset(const String& path)
		{
			File* pAssetFile = Engine::GetFilesystem().GetFile(path);

			if (!pAssetFile)
			{
				LogError("Error loading asset [%s]. File does not exist.", path.Str());
				return nullptr;
			}

			return GetOrLoadAsset<AssetType>(*pAssetFile);
		}

		template<typename AssetType>
		bool UnloadAsset(AssetType* pAsset)
		{
			if (!pAsset || !pAsset->GetSourceFile())
			{
				return false;
			}

			const String ext = pAsset->GetSourceFile()->GetExtention();
			AssetLoader* pLoader;
			bool result = true;

			auto& loaderIt = mLoaders.Find(ext);
			if (loaderIt != mLoaders.End())
			{
				pLoader = loaderIt->value;
			}
			else
			{
				// No loader
				return false;
			}

			result = pLoader->UnloadAsset(pAsset);
			mAssets.Remove(pAsset->GetSourceFile()->GetPath());
			
			return result;
		}
	};
}