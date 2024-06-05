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
		DWORD dwAccess		= 0;
		DWORD dwCreation	= OPEN_ALWAYS;
		DWORD dwAttrribs	= FILE_ATTRIBUTE_NORMAL;
		DWORD dwShareMode	= 0;

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

		if (openFlags & FILE_OPEN_APPEND)
		{
			dwAccess |= FILE_APPEND_DATA;
		}

		if (openFlags & FILE_OPEN_CLEAR)
		{
			dwCreation = TRUNCATE_EXISTING;
		}

		if (openFlags & FILE_OPEN_CREATE)
		{
			dwCreation = CREATE_NEW;
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

		if (openFlags & FILE_OPEN_SHARE_READ)
		{
			dwShareMode |= FILE_SHARE_READ;
			fileFlags |= FILE_SHARED;
		}

		if (openFlags & FILE_OPEN_SHARE_WRITE)
		{
			dwShareMode |= FILE_SHARE_WRITE;
			fileFlags |= FILE_SHARED;
		}

		if (dwAttrribs != FILE_ATTRIBUTE_NORMAL)
		{
			// Ensure dwAttrribs does not contain FILE_ATTRIBUTE_NORMAL if not normal
			dwAttrribs &= ~FILE_ATTRIBUTE_NORMAL;
		}

		HANDLE pFileHandle = ::CreateFile(file.GetPath().Str(),
			dwAccess, dwShareMode, NULL, dwCreation, dwAttrribs, NULL);

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

#undef CreateFile

	bool WinApiFilesystemHandler::CreateFile(const String& path, File*& pOutFile, FileOpenFlags openFlags)
	{
		File* pFile = mpFileAllocator->Allocate(path, *this, (void*)NULL, (FileFlags)FILE_VALID, (uSize)0, (uSize)0);

		void* pNativeHandle = nullptr;
		FileFlags fileFlags = 0;

		if (!OpenFile(*pFile, openFlags, pNativeHandle, fileFlags))
		{
			mpFileAllocator->Free(pFile);
			return false;
		}

		CloseFile(*pFile, pNativeHandle, fileFlags);

		pOutFile = pFile;

		return true;
	}

	bool WinApiFilesystemHandler::CreateFolder(const String& path, Folder*& pOutFolder, uSize priority)
	{
		Folder* pFolder = mpFolderAllocator->Allocate(path, *this, (void*)NULL, (FolderFlags)FOLDER_VALID, priority);

		pOutFolder = pFolder;

		return true;
	}

	bool WinApiFilesystemHandler::DeleteFolder(Folder& folder)
	{
		return false;
	}

	// @TODO/NOTE: Will not work in 32-bit systems
	bool WinApiFilesystemHandler::MapFile(File& file, uInt8*& pOutMapPtr, uInt64 reserveBytes, FileMapFlags mapFlags)
	{
		DWORD hiSize = reserveBytes >> 32;
		DWORD loSize = reserveBytes & 0xFFFFFFFF;

		DWORD dwPageAccess = 0;
		DWORD dwMapAccess = 0;

#define SYS_FILE_MAP_READ FILE_MAP_READ
#define SYS_FILE_MAP_WRITE FILE_MAP_WRITE
#undef FILE_MAP_READ
#undef FILE_MAP_WRITE

		if (mapFlags & FILE_MAP_READ)
		{
			dwMapAccess = SYS_FILE_MAP_READ;
			dwPageAccess = PAGE_READONLY;
		}
		
		if (mapFlags & FILE_MAP_WRITE)
		{
			// @NOTE: FILE_MAP_WRITE includes FILE_MAP_READ
			dwMapAccess = SYS_FILE_MAP_WRITE;
			dwPageAccess = PAGE_READWRITE;
		}

#define FILE_MAP_READ SYS_FILE_MAP_READ
#define FILE_MAP_WRITE SYS_FILE_MAP_WRITE
#undef SYS_FILE_MAP_READ
#undef SYS_FILE_MAP_WRITE

		HANDLE mapHandle = CreateFileMappingA((HANDLE)file.GetNativeHandle(), NULL, dwPageAccess, hiSize, loSize, NULL);

		DWORD mapError = GetLastError();

		if (mapError == ERROR_ALREADY_EXISTS)
		{
			// TODO
		}

		if (!mapHandle)
		{
			WinApiPrintError();
			return false;
		}

		void* pMapPtr = MapViewOfFile(mapHandle, dwMapAccess, 0, 0, 0);

		if (!pMapPtr)
		{
			WinApiPrintError();
			CloseHandle(mapHandle);
			return false;
		}

		pOutMapPtr = (uInt8*)pMapPtr;

		CloseHandle(mapHandle);

		return true;
	}

	bool WinApiFilesystemHandler::UnmapFile(File& file)
	{
		return true;
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

	bool WinApiFilesystemHandler::SetFilePtr(File& file, int64 offset, FilePtrRelative relative, uInt64& outIntPtr)
	{
		LONG dwHiFilePointer = 0;
		DWORD dwLoFilePointer = 0;

		DWORD dwMoveMethod = FILE_BEGIN;

		if (relative == FILE_PTR_CURRENT)
		{
			dwMoveMethod = FILE_CURRENT;
		}
		else if (relative == FILE_PTR_END)
		{
			dwMoveMethod = FILE_END;
		}

		dwLoFilePointer = SetFilePointer((HANDLE)file.GetNativeHandle(), offset, &dwHiFilePointer, dwMoveMethod);

		if (dwLoFilePointer == INVALID_SET_FILE_POINTER)
		{
			WinApiPrintError();
			return false;
		}

		outIntPtr = ((uInt64)dwHiFilePointer << 32 | dwLoFilePointer);

		return true;
	}

	bool WinApiFilesystemHandler::GetFilePtr(const File& file, uInt64& outIntPtr)
	{
		return SetFilePtr(const_cast<File&>(file), 0, FILE_PTR_CURRENT, outIntPtr);
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