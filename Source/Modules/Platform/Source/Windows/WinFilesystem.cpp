#include "Windows/WinFilesystem.h"

#include "Windows/WinApi.h"
#include "Log.h"

namespace Quartz
{
	bool WinApiPopulateFolder(const String& rootPath, const String& path, 
		Folder& outFolder, PoolAllocator<File>& fileAllocator)
	{
		String folderPath = rootPath + "/" + path + "/*";

		if (path.IsEmpty())
		{
			folderPath = rootPath + "/*";
		}

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

				String subFolderPath = path + "/" + subFolderName;

				if (path.IsEmpty())
				{
					subFolderPath = subFolderName;
				}

				Folder& subFolder = outFolder.folders.PushBack(Folder());
				subFolder.name = subFolderName;
				subFolder.path = subFolderPath;
				subFolder.flags |= FOLDER_VALID;

				if (!outFolder.folders.Contains(subFolder))
				{
					outFolder.folders.PushBack(subFolder);
				}
			}
			else
			{
				String fileName = fileData.cFileName;
				uInt64 sizeBytes = ((uInt64)fileData.nFileSizeHigh << sizeof(DWORD) * 8) | fileData.nFileSizeLow;

				FileFunctions fileFunctions = {};
				fileFunctions.openFunc	= WinApiOpenFile;
				fileFunctions.closeFunc = WinApiCloseFile;
				fileFunctions.readFunc	= WinApiReadFile;
				fileFunctions.writeFunc = WinApiWriteFile;

				String filePath = path + "/" + fileName;

				if (path.IsEmpty())
				{
					filePath = fileName;
				}

				File* pFile = fileAllocator.Allocate(filePath, (void*)NULL, (FileFlags)FILE_VALID, (uSize)sizeBytes, (uSize)0, fileFunctions);

				outFolder.files.PushBack(pFile);
			}
		}
		while (FindNextFile(nextHandle, &fileData));

		return true;
	}

	bool WinApiOpenFile(File& file, FileOpenFlags openFlags, void*& pOutHandle, FileFlags& outFlags)
	{
		DWORD dwAccess = NULL;
		DWORD dwCreation = OPEN_EXISTING;
		DWORD dwAttrribs = FILE_ATTRIBUTE_NORMAL;

		FileFlags fileFlags = FILE_VALID;

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

		HANDLE pFileHandle = CreateFile(file.Path().Str(), 
			dwAccess, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, 
			dwCreation, dwAttrribs, NULL);

		if (pFileHandle == INVALID_HANDLE_VALUE)
		{
			WinApiPrintError();
			return false;
		}

		pOutHandle = pFileHandle;

		return true;
	}

	bool WinApiCloseFile(File& file)
	{
		return CloseHandle((HANDLE)file.GetNativeHandle());
	}

	bool WinApiReadFile(File& file, void* pOutData, uSize sizeBytes)
	{
		DWORD dwBytesRead = 0;
		BOOL result = ReadFile(file.GetNativeHandle(), pOutData, sizeBytes, &dwBytesRead, NULL);

		if (!result || dwBytesRead != sizeBytes)
		{
			WinApiPrintError();
			return false;
		}

		return true;
	}

	bool WinApiWriteFile(File& file, void* pInData, uSize sizeBytes)
	{
		DWORD dwBytesWritten = 0;
		BOOL result = WriteFile(file.GetNativeHandle(), pInData, sizeBytes, &dwBytesWritten, NULL);

		if (!result || dwBytesWritten != sizeBytes)
		{
			WinApiPrintError();
			return false;
		}

		return true;
	}
}