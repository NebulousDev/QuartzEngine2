#include "Windows/WinFilesystem.h"

#include "Windows/WinApi.h"
#include "Log.h"

namespace Quartz
{
	WinApiFilesystemHandler::WinApiFilesystemHandler(Allocator* pAllocator)
	{
		mpFileAllocator = new PoolAllocator<File>(8192 * sizeof(File), pAllocator);
		mpFolderAllocator = new PoolAllocator<Folder>(8192 * sizeof(Folder), pAllocator);
	}

	WinApiFilesystemHandler::~WinApiFilesystemHandler()
	{
		delete mpFileAllocator;
		delete mpFolderAllocator;
	}

	bool WinApiFilesystemHandler::OpenFile(File& file, FileOpenFlags openFlags, void*& pOutHandle, FileFlags& outFlags)
	{
		DWORD dwAccess = NULL;
		DWORD dwCreation = OPEN_EXISTING;
		DWORD dwAttrribs = FILE_ATTRIBUTE_NORMAL;

		FileFlags fileFlags = FILE_VALID | FILE_OPEN;

		if (openFlags & FILE_OPEN_READ)
		{
			dwAccess |= GENERIC_READ;
			fileFlags |= FILE_READ;
		}

		if (openFlags & FILE_OPEN_WRITE)
		{
			dwAccess |= GENERIC_WRITE;
			fileFlags |= FILE_WRITE;
		}

		if (openFlags & FILE_OPEN_CREATE)
		{
			dwCreation |= CREATE_NEW;
		}

		if (openFlags & FILE_OPEN_HIDDEN)
		{
			dwAttrribs |= FILE_ATTRIBUTE_HIDDEN;
			fileFlags |= FILE_HIDDEN;
		}

		if (openFlags & FILE_OPEN_TEMPORARY)
		{
			dwAttrribs |= (FILE_ATTRIBUTE_TEMPORARY | FILE_FLAG_DELETE_ON_CLOSE);
			fileFlags |= FILE_HIDDEN;
		}

		if (dwAttrribs != FILE_ATTRIBUTE_NORMAL)
		{
			// Ensure dwAttrribs does not contain FILE_ATTRIBUTE_NORMAL if not normal
			dwAttrribs &= ~FILE_ATTRIBUTE_NORMAL;
		}

		HANDLE pFileHandle = CreateFile(file.GetPath().Str(),
			dwAccess, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, 
			dwCreation, dwAttrribs, NULL);

		if (pFileHandle == INVALID_HANDLE_VALUE)
		{
			WinApiPrintError();
			return false;
		}

		pOutHandle	= pFileHandle;
		outFlags	= fileFlags;

		return true;
	}

	bool WinApiFilesystemHandler::CloseFile(File& file, void*& pOutHandle, FileFlags& outFlags)
	{
		if (CloseHandle((HANDLE)file.GetNativeHandle()))
		{
			pOutHandle = nullptr;
			outFlags = file.GetFlags() & ~FILE_OPEN;
			return true;
		}
		
		return false;
	}

	bool WinApiFilesystemHandler::OpenFolder(Folder& folder, void*& pOutHandle)
	{
		return false;
	}

	bool WinApiFilesystemHandler::CloseFolder(Folder& folder, void*& pOutHandle)
	{
		return false;
	}


	//bool WinApiFilesystemHandler::CreateFile() const = 0;
	//bool WinApiFilesystemHandler::DeleteFile() const = 0;

	bool WinApiFilesystemHandler::CreateFolder(const String& path, Folder*& pOutFolder, uSize priority)
	{
		Folder* pSubFolder = mpFolderAllocator->Allocate(path, *this, (void*)NULL, (FolderFlags)FOLDER_VALID, priority);

		pOutFolder = pSubFolder;

		return true;
	}

	bool WinApiFilesystemHandler::DeleteFolder(Folder& folder)
	{
		return false;
	}

	bool WinApiFilesystemHandler::ReadFile(File& file, void* pOutData, uSize sizeBytes)
	{
		DWORD dwBytesRead = 0;
		BOOL result = ::ReadFile((HANDLE)file.GetNativeHandle(), pOutData, sizeBytes, &dwBytesRead, NULL);

		if (!result || dwBytesRead != sizeBytes)
		{
			WinApiPrintError();
			return false;
		}

		return true;
	}

	bool WinApiFilesystemHandler::WriteFile(File& file, void* pInData, uSize sizeBytes)
	{
		DWORD dwBytesWritten = 0;
		BOOL result = ::WriteFile((HANDLE)file.GetNativeHandle(), pInData, sizeBytes, &dwBytesWritten, NULL);

		if (!result || dwBytesWritten != sizeBytes)
		{
			WinApiPrintError();
			return false;
		}

		return true;
	}

	bool WinApiFilesystemHandler::PopulateChildren(Folder& folder, const Filesystem& filesystem,
		Array<Folder*>& outFolders, Array<File*>& outFiles)
	{
		String folderPath = folder.GetPath() + "/*";

		WIN32_FIND_DATA fileData;
		HANDLE nextHandle = FindFirstFile(folderPath.Str(), &fileData);

		if (nextHandle == INVALID_HANDLE_VALUE)
		{
			WinApiPrintError();
			return false;
		}

		do
		{
			if (fileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			{
				String subFolderName = fileData.cFileName;

				if (subFolderName == "." || subFolderName == "..")
				{
					continue;
				}

				String subFolderPath = folder.GetPath() + "/" + subFolderName;

				// @TODO: find the correct handler rather than using the same one vvv
				Folder* pSubFolder = mpFolderAllocator->Allocate(subFolderPath, *this, (void*)NULL, (FolderFlags)FOLDER_VALID, folder.GetPriority());

				outFolders.PushBack(pSubFolder);
			}
			else
			{
				String fileName = fileData.cFileName;
				uInt64 sizeBytes = ((uInt64)fileData.nFileSizeHigh << sizeof(DWORD) * 8) | fileData.nFileSizeLow;

				String filePath = folder.GetPath() + "/" + fileName;

				File* pFile = mpFileAllocator->Allocate(filePath, *this, (void*)NULL, (FileFlags)FILE_VALID, (uSize)sizeBytes, (uSize)0);

				outFiles.PushBack(pFile);
			}
		}
		while (FindNextFile(nextHandle, &fileData));

		return true;
	}

	bool WinApiFilesystemHandler::IsVirtual() const
	{
		return false;
	}

	bool WinApiCheckChanges(const Folder& rootFolder, const Stack<File*>& changedFiles)
	{
		//ReadDirectoryChangesW
		return false;
	}
}