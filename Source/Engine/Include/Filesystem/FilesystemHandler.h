#pragma once

#include "../EngineAPI.h"
#include "Types/String.h"
#include "Types/Array.h"

namespace Quartz
{
	class File;
	class Folder;
	class Filesystem;

	using FileOpenFlags = uInt16;
	using FileMapFlags = uInt16;
	using FileFlags = uInt16;

	class QUARTZ_ENGINE_API FilesystemHandler
	{
	public:
		virtual bool OpenFile(File& file, FileOpenFlags openFlags, void*& pOutHandle, FileFlags& outFlags) = 0;
		virtual bool CloseFile(File& file, void*& pOutHandle, FileFlags& outFlags) = 0;
		virtual bool OpenFolder(Folder& folder, void*& pOutHandle) = 0;
		virtual bool CloseFolder(Folder& folder, void*& pOutHandle) = 0;

		virtual bool CreateFile(const String& path, File*& pOutFile, FileOpenFlags openFlags) = 0;
		//virtual bool DeleteFile() const = 0;
		virtual bool CreateFolder(const String& path, Folder*& pOutFolder, uSize priority) = 0;
		virtual bool DeleteFolder(Folder& Folder) = 0;

		virtual bool MapFile(File& file, uInt8*& pOutMapPtr, uInt64 reserveBytes, FileMapFlags mapFlags) = 0;
		virtual bool UnmapFile(File& file) = 0;

		virtual bool ReadFile(File& file, void* pOutData, uSize sizeBytes) = 0;
		virtual bool WriteFile(File& file, void* pInData, uSize sizeBytes) = 0;

		virtual bool PopulateChildren(Folder& folder, const Filesystem& filesystem,
			Array<Folder*>& outFolders, Array<File*>& outFiles) = 0;

		virtual bool IsVirtual() const = 0;
	};
}