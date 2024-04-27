#include "Filesystem/Filesystem.h"

#include "Log.h"
#include "assert.h"

namespace Quartz
{
	void FilesystemImpl::SetPopulateFolderFunc(PopulateFolderFunc polulateFunc)
	{
		mPopulateFolderFunc = polulateFunc;
	}

	bool Filesystem::PopulateSubfolders(const String& rootPath, Folder& folder)
	{
		bool anyError = false;

		for (Folder& subFolder : folder.folders)
		{
			String folderPath = folder.path + "/" + subFolder.name;

			if (folder.path.IsEmpty())
			{
				folderPath = subFolder.name;
			}

			if (!mPopulateFolderFunc(rootPath, folderPath, subFolder, mFileAllocator))
			{
				LogWarning("Failed to read directory [%s]!", folderPath.Str());
				anyError = true;
			}

			if (!PopulateSubfolders(rootPath, subFolder))
			{
				anyError = true;
			}
		}

		for (File* pFile : folder.files)
		{
			mFileMap.Put(pFile->Path(), pFile);
		}

		return !anyError;
	}

	Filesystem::Filesystem() :
		mFileAllocator(8192 * sizeof(File))
	{
		mFileMap.Reserve(8192);
	}

	bool Filesystem::AddRoot(const String& rootPath, uSize priority)
	{
		// @TODO: check valid root

		RootFolder& rootFolder = mRoots.PushBack(RootFolder());
		
		if (!mPopulateFolderFunc(rootPath, String(), rootFolder, mFileAllocator))
		{
			LogError("Failed to add root directory [%s]!", rootPath.Str());
			mRoots.FastRemove(rootFolder);
			return false;
		}

		PopulateSubfolders(rootPath, rootFolder);

		// Note: must be assigned after PopulateSubfolders 
		// to keep the root path out of the the file paths
		rootFolder.path		= rootPath;
		rootFolder.name		= rootPath; // TODO
		rootFolder.priority = priority;

		return true;
	}

	const Folder* Filesystem::GetFolder(const String& folderPath)
	{
		return nullptr; // @TODO
	}

	File* Filesystem::GetFile(const String& path)
	{
		auto& fileIt = mFileMap.Find(path);
		if (fileIt != mFileMap.End())
		{
			File* pFile = fileIt->value;

			if (!pFile->IsValid())
			{
				return nullptr;
			}

			return pFile;
		}

		return nullptr;
	}
}