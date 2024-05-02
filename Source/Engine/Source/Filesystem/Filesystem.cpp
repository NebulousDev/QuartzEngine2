#include "Filesystem/Filesystem.h"

#include "Filesystem/File.h"
#include "Filesystem/Folder.h"

#include "Log.h"
#include "assert.h"

namespace Quartz
{
	//bool Filesystem::PopulateSubfolders(const String& rootPath, Folder& folder)
	//{
	//	bool anyError = false;
	//
	//	for (Folder* pSubFolder : folder.mFolders)
	//	{
	//		String folderPath = pSubFolder->mPath + "/" + pSubFolder->Name();
	//
	//		if (folder.mPath.IsEmpty())
	//		{
	//			folderPath = pSubFolder->Name();
	//		}
	//
	//		if (!mPopulateFolderFunc(rootPath, folderPath, *pSubFolder, mFileAllocator, mFolderAllocator))
	//		{
	//			LogWarning("Failed to read directory [%s]!", folderPath.Str());
	//			anyError = true;
	//		}
	//
	//		if (!PopulateSubfolders(rootPath, *pSubFolder))
	//		{
	//			anyError = true;
	//		}
	//	}
	//
	//	for (File* pFile : folder.mFiles)
	//	{
	//		mFileMap.Put(pFile->Path(), pFile);
	//	}
	//
	//	return !anyError;
	//}

	Filesystem::Filesystem()
	{
		mFileMap.Reserve(8192 * 2);
	}

	void Filesystem::PopulateRecursive(Folder& folder, FilesystemHandler& handler)
	{
		handler.PopulateChildren(folder, *this, folder.mFolders, folder.mFiles);

		for (Folder* pSubFolder : folder.mFolders)
		{
			PopulateRecursive(*pSubFolder, handler);
		}

		for (File* pFile : folder.mFiles)
		{
			mFileMap.Put(pFile->Path(), pFile);
		}
	}

	bool Filesystem::AddRoot(const String& rootPath, FilesystemHandler& handler, uSize priority)
	{
		// @TODO: check valid root

		Folder* pRootFolder;

		if(!handler.CreateFolder(rootPath, pRootFolder, priority))
		{
			LogError("Failed to add root directory [%s]!", rootPath.Str());
			return false;
		}
	
		PopulateRecursive(*pRootFolder, handler);

		mRootFolders.PushBack(pRootFolder);

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
		else
		{
			for (Folder* pRoot : mRootFolders)
			{
				const String& fullPath = pRoot->Path() + "/" + path;
				fileIt = mFileMap.Find(fullPath);
				if (fileIt != mFileMap.End())
				{
					File* pFile = fileIt->value;

					if (!pFile->IsValid())
					{
						return nullptr;
					}

					return pFile;
				}
			}
		}

		return nullptr;
	}
	
	bool Filesystem::CheckForChanges()
	{
		return false;
	}
}