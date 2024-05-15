#pragma once

#include "Filesystem/File.h"
#include "Filesystem/Folder.h"
#include "Filesystem/Filesystem.h"

namespace Quartz
{
#ifdef CreateFile
#define SysCreateFile CreateFile
#undef CreateFile
#endif

	class WinApiFilesystemHandler : public FilesystemHandler
	{
	private:
		PoolAllocator<File>*	mpFileAllocator;
		PoolAllocator<Folder>*	mpFolderAllocator;

	public:
		WinApiFilesystemHandler(Allocator* pAllocator);
		~WinApiFilesystemHandler();

		bool OpenFile(File& file, FileOpenFlags openFlags, void*& pOutHandle, FileFlags& outFlags) override;
		bool CloseFile(File& file, void*& pOutHandle, FileFlags& outFlags) override;
		bool OpenFolder(Folder& folder, void*& pOutHandle) override;
		bool CloseFolder(Folder& folder, void*& pOutHandle) override;

		bool CreateFile(const String& path, File*& pOutFile, FileOpenFlags openFlags) override;
		//bool DeleteFile() const = 0;
		bool CreateFolder(const String& path, Folder*& pOutFolder, uSize priority) override;
		bool DeleteFolder(Folder& Folder) override;

		bool ReadFile(File& file, void* pOutData, uSize sizeBytes) override;
		bool WriteFile(File& file, void* pInData, uSize sizeBytes) override;

		bool PopulateChildren(Folder& folder, const Filesystem& filesystem,
			Array<Folder*>& outFolders, Array<File*>& outFiles) override;

		bool IsVirtual() const override;
	};

#ifdef SysCreateFile
#define CreateFile SysCreateFile
#endif
}