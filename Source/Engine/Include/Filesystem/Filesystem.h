#pragma once

#include "../EngineAPI.h"

#include "File.h"
#include "Folder.h"

#include "Types/Array.h"
#include "Types/Map.h"
#include "Memory/PoolAllocator.h"

namespace Quartz
{
	class QUARTZ_ENGINE_API Filesystem
	{
	public:
		using PopulateFolderFunc = bool (*)(const String& rootPath, const String& path, 
			Folder& outFolder, PoolAllocator<File>& fileAllocator);

	protected:
		PoolAllocator<File>	mFileAllocator;
		Array<RootFolder>	mRoots;
		Map<String, File*>	mFileMap;

	protected:
		PopulateFolderFunc	mPopulateFolderFunc;

		bool PopulateSubfolders(const String& rootPath, Folder& folder);

	public:
		Filesystem();

		bool AddRoot(const String& rootPath, uSize priority);

		const Folder* GetFolder(const String& folderPath);
		File* GetFile(const String& path);
	};

	class QUARTZ_ENGINE_API FilesystemImpl : public Filesystem
	{
	public:
		void SetPopulateFolderFunc(PopulateFolderFunc polulateFunc);
	};
}