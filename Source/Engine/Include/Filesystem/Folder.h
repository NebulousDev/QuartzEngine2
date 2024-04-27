#pragma once

#include "Filesystem/File.h"
#include "Types/Array.h"

namespace Quartz
{
	enum FolderFlagBits
	{
		FOLDER_VALID	= 0x1,
		FOLDER_VIRTUAL	= 0x2
	};

	using FolderFlags = uSize;

	struct QUARTZ_ENGINE_API Folder
	{
		String			name;	// @TODO: make substring idx
		String			path;
		Array<Folder>	folders;
		Array<File*>	files;
		FolderFlags		flags;

		friend QUARTZ_ENGINE_API bool operator==(const Folder& folder0, const Folder& folder1);

		bool IsValid() const { return flags & FOLDER_VALID; }
		bool IsVirtual() const { return flags & FOLDER_VIRTUAL; }
	};

	struct QUARTZ_ENGINE_API RootFolder : public Folder
	{
		uSize priority;
	};
}