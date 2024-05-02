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
		Stack<File*>			mChangedFiles;

		void PopulateRecursive(Folder& folder, FilesystemHandler& handler);

	public:
		Filesystem();

		bool AddRoot(const String& rootPath, FilesystemHandler& handler, uSize priority);

		const Folder* GetFolder(const String& folderPath);
		File* GetFile(const String& path);

		bool CheckForChanges();
	};
}