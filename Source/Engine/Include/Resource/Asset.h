#pragma once

#include "EngineAPI.h"
#include "Filesystem/File.h"

namespace Quartz
{
	using AssetID = uInt64;

	class QUARTZ_ENGINE_API Asset
	{
	public:
		friend class AssetManager;

	protected:
		File*	mpSourceFile;
		AssetID	mAssetId;
		bool	mLoaded;

	public:
		Asset() = default;
		inline Asset(File* pSourceFile) : mpSourceFile(pSourceFile) {}

		inline File*		GetSourceFile() { return mpSourceFile; }
		inline AssetID		GetAssetID() const { return mAssetId; }
		inline bool			IsLoaded() const { return mLoaded; }
		inline Substring	GetName() const { return mpSourceFile->GetName(); }
		inline String		GetPath() const { return mpSourceFile->GetPath(); }
	};
}