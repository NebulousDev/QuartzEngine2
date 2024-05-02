#include "Filesystem/Folder.h"

namespace Quartz
{
	Folder::Folder(const String& path, Handler& handler, void* pHandle, FolderFlags flags, uSize priority) :
		mPath(path), mpHandler(&handler), mpNative(pHandle), mFlags(flags), mPriority(priority)
	{
		mNameIdx = path.FindReverse("/");

		if (mNameIdx == 0) // @TODO: ensure this isn't needed
		{
			mNameIdx = path.FindReverse("\\");
		}
	}

	bool operator==(const Folder& folder0, const Folder& folder1)
	{
		return folder0.mPath == folder1.mPath;
	}

	bool operator!=(const Folder& folder0, const Folder& folder1)
	{
		return !operator==(folder0, folder1);
	}
}