#pragma once

#include "EngineAPI.h"
#include "Types/String.h"
#include "Types/Array.h"

namespace Quartz
{
	class File;

	enum FolderFlagBits
	{
		FOLDER_VALID	= 0x1,
		FOLDER_VIRTUAL	= 0x2,
		FOLDER_IS_ROOT	= 0x4
	};

	using FolderFlags = uSize;

	class QUARTZ_ENGINE_API Folder
	{
	public:
		friend class Filesystem;
		friend class FilesystemHandler;

		using Handler = FilesystemHandler;

	protected:
		String			mPath;
		uInt32			mNameIdx;
		FolderFlags		mFlags;
		uSize			mPriority;

		Array<Folder*>	mFolders;
		Array<File*>	mFiles;

		void*			mpNative;
		Handler*		mpHandler;

	public:
		Folder() = default;
		Folder(const String& path, Handler& handler, void* pHandle, FolderFlags flags, uSize priority = 0);

		friend QUARTZ_ENGINE_API bool operator==(const Folder& folder0, const Folder& folder1);
		friend QUARTZ_ENGINE_API bool operator!=(const Folder& folder0, const Folder& folder1);

		const Substring Name() const { return mPath.Substring(mNameIdx); }
		const String& Path() const { return mPath; }
		FolderFlags Flags() const { return mFlags; }
		const Array<Folder*>& GetChildFolders() const { return mFolders; }
		const Array<File*>& GetChildFiles() const { return mFiles; }
		bool IsValid() const { return mFlags & FOLDER_VALID; }
		bool IsVirtual() const { return mFlags & FOLDER_VIRTUAL; }
		void* GetNativeHandle() const { return mpNative; }
		uSize GetPriority() const { return mPriority; }
	};
}