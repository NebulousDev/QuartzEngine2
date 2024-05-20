#pragma once

#include "AssetHandler.h"
#include "Filesystem/Filesystem.h"
#include "Engine.h"
#include "Log.h"
#include "Types/Map.h"

namespace Quartz
{
	class QUARTZ_ENGINE_API AssetManager
	{
	private:
		Map<String, AssetHandler*>	mHandlers;
		Map<String, Asset*>			mAssets;
		AssetID						mNextAssetID; // @TODO: find a better system

	public:
		AssetManager() :
			mHandlers(128), mAssets(8196), mNextAssetID(1) {}

		template<typename AssetHandlerType>
		bool RegisterAssetHandler(const String& ext, AssetHandlerType* pAssetHandler)
		{
			mHandlers.Put(ext, static_cast<AssetHandler*>(pAssetHandler));
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
			AssetHandler* pHandler = nullptr;
			Asset* pAsset = nullptr;

			auto& loaderIt = mHandlers.Find(ext);
			if (loaderIt != mHandlers.End())
			{
				pHandler = loaderIt->value;
			}
			else
			{
				LogError("Error loading asset [%s]. No registered loader is associated with the extension '%s'.", 
					assetFile.GetPath().Str(), ext.Str());
				return nullptr;
			}

			if (!pHandler->LoadAsset(assetFile, pAsset))
			{
				// Error message in LoadAsset()
				return nullptr;
			}

			// @TODO: find a better system
			pAsset->mAssetId = mNextAssetID;
			mNextAssetID++;

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
			AssetHandler* pHandler;
			bool result = true;

			auto& loaderIt = mHandlers.Find(ext);
			if (loaderIt != mHandlers.End())
			{
				pHandler = loaderIt->value;
			}
			else
			{
				// No loader
				return false;
			}

			result = pHandler->UnloadAsset(pAsset);
			mAssets.Remove(pAsset->GetSourceFile()->GetPath());
			
			return result;
		}
	};
}