#include "Filesystem/Filesystem.h"

#include "Filesystem/File.h"
#include "Filesystem/Folder.h"

#include "Log.h"
#include "assert.h"

namespace Quartz
{
	Filesystem::Filesystem()
	{
		mFileMap.Reserve(8192 * 2);
		mFolderMap.Reserve(8192 * 2);
	}

	void Filesystem::PopulateRecursive(const String& rootPath, Folder& folder, FilesystemHandler& handler)
	{
		bool isRoot = rootPath == folder.mPath;

		handler.PopulateChildren(folder, *this, folder.mFolders, folder.mFiles);

		for (Folder* pSubFolder : folder.mFolders)
		{
			PopulateRecursive(rootPath, *pSubFolder, handler);
		}

		Folder* pPriorityFolder = nullptr;
		String trimmedFolderPath = rootPath;
		sSize maxPriority = -1;

		if (!isRoot)
		{
			trimmedFolderPath = folder.mPath.Substring(rootPath.Length() + 1);
		}
		else
		{
			pPriorityFolder = GetFolder(trimmedFolderPath);

			if (!pPriorityFolder)
			{
				for (Folder* pRoot : mRootFolders)
				{
					uSize trimIdx = trimmedFolderPath.Find(pRoot->mPath);

					if (trimIdx >= trimmedFolderPath.Length())
					{
						continue;
					}

					String rootTrimmedFolderPath = trimmedFolderPath.Substring(trimIdx + pRoot->mPath.Length() + 1);
					Folder* pFoundFolder = GetFolder(rootTrimmedFolderPath);

					if (pFoundFolder && pFoundFolder->mPriority > maxPriority)
					{
						pPriorityFolder = pFoundFolder;
						maxPriority = pFoundFolder->mPriority;
					}
				}
			}

		}
			
		if (!pPriorityFolder || pPriorityFolder->mPriority < folder.mPriority)
		{
			// Only override files if the parent folder is of higher priority

			for (File* pFile : folder.mFiles)
			{
				String trimmedFilePath = pFile->mPath.Substring(rootPath.Length() + 1);
				mFileMap.Put(trimmedFilePath, pFile);
			}

			if (!isRoot)
			{
				mFolderMap.Put(trimmedFolderPath, &folder);
			}
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
	
		PopulateRecursive(rootPath, *pRootFolder, handler);

		mRootFolders.PushBack(pRootFolder);

		return true;
	}

	File* Filesystem::GetFile(const String& filePath)
	{
		auto& fileIt = mFileMap.Find(filePath);
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

	Folder* Filesystem::GetFolder(const String& folderPath)
	{
		auto& folderIt = mFolderMap.Find(folderPath);
		if (folderIt != mFolderMap.End())
		{
			Folder* pFolder = folderIt->value;

			if (!pFolder->IsValid())
			{
				return nullptr;
			}

			return pFolder;
		}

		return nullptr;
	}

	File* Filesystem::CreateFile(const String& filePath)
	{
		File* pFoundFile = GetFile(filePath);
		if (pFoundFile)
		{
			return pFoundFile;
		}

		if (mRootFolders.Size() == 0)
		{
			LogError("Error creating file [%s]. No root folders assigned.", filePath.Str());
			return nullptr;
		}

		FilesystemHandler* pHandler = nullptr;
		String fullPath;

		uSize folderPathIndexF = filePath.FindReverse("/");
		uSize folderPathIndexB = filePath.FindReverse("\\");

		uSize folderPathIndex = 
			folderPathIndexF > folderPathIndexB ? folderPathIndexF : folderPathIndexB;

		Substring folderPath = filePath.Substring(0, folderPathIndex);
		Substring fileName = filePath.Substring(folderPathIndex + 1);

		Folder* pFolder = GetFolder(folderPath);

		if (!pFolder)
		{
			LogError("Error creating file [%s]. No valid root folder contains the path '%s'.",
				filePath.Str(), folderPath.Str());
			return nullptr;
		}

		pHandler = pFolder->mpHandler;
		fullPath = pFolder->mPath + "/" + fileName;

		if (pHandler == nullptr)
		{
			LogError("Error creating file [%s]. No valid root folder contains the path '%s'.", 
				filePath.Str(), folderPath.Str());
			return nullptr;
		}

		File* pFile = nullptr;

		if (!pHandler->CreateFile(fullPath, pFile, 0))
		{
			LogError("Error creating file [%s]. FilesystemHandler failed to create the file.", filePath.Str());
			return nullptr;
		}

		mFileMap.Put(filePath, pFile);

		return pFile;
	}

	bool Filesystem::DeleteFile(File& file)
	{
		return false;
	}
	
	bool Filesystem::FileExists(const String& filePath)
	{
		return GetFile(filePath) != nullptr;
	}

	bool Filesystem::FolderExists(const String& folderPath)
	{
		return GetFolder(folderPath) != nullptr;
	}

	bool Filesystem::CheckForChanges()
	{
		return false;
	}
}