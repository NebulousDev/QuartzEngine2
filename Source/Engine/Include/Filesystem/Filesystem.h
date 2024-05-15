#pragma once

#include "../EngineAPI.h"
#include "FilesystemHandler.h"

#include "Types/Array.h"
#include "Types/Stack.h"
#include "Types/Map.h"
#include "Types/String.h"
#include "Memory/PoolAllocator.h"

namespace Quartz
{
	class File;
	class Folder;

	class QUARTZ_ENGINE_API Filesystem
	{
	protected:
		Array<Folder*>			mRootFolders;
		Map<String, File*>		mFileMap;
		Map<String, Folder*>	mFolderMap;
		Stack<File*>			mChangedFiles;

		void PopulateRecursive(const String& rootPath, Folder& folder, FilesystemHandler& handler);

	public:
		Filesystem();

		bool AddRoot(const String& rootPath, FilesystemHandler& handler, uSize priority);

		File* GetFile(const String& filePath);
		Folder* GetFolder(const String& folderPath);

		File* CreateFile(const String& filePath);
		bool DeleteFile(File& file);

		bool FileExists(const String& filePath);
		bool FolderExists(const String& folderPath);

		bool CheckForChanges();
	};
}