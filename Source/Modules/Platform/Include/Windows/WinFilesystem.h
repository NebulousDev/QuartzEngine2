#pragma once

#include "Filesystem/Filesystem.h"

namespace Quartz
{
	bool WinApiPopulateFolder(const String& rootPath, const String& path, 
		Folder& outFolder, PoolAllocator<File>& fileAllocator);

	bool WinApiOpenFile(File& file, FileOpenFlags openFlags, void*& pOutHandle, FileFlags& outFlags);
	bool WinApiCloseFile(File& file);

	bool WinApiReadFile(File& file, void* pOutData, uSize sizeBytes);
	bool WinApiWriteFile(File& file, void* pInData, uSize sizeBytes);
}