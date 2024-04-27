#include "Filesystem/File.h"

#include "Log.h"

namespace Quartz
{
	File::File() : 
		mPath(), mFlags(FileFlags(0)), mpHandle(nullptr),
		mSizeBytes(0), mOffsetBytes(0), mFuncs() {};

	File::File(const String& path,void* pHandle, FileFlags flags, 
		uSize sizeBytes, uSize offsetBytes, FileFunctions funcs) :
		mPath(path), mFlags(flags), mpHandle(pHandle), 
		mSizeBytes(sizeBytes), mOffsetBytes(offsetBytes), mFuncs(funcs)
	{
		uSize extIdx = path.FindReverse(".");

		if (extIdx != 0)
		{
			mExtIdx = extIdx + 1;
		}
		else
		{
			LogWarning("File [%s] has no file extention.", path.Str());
			mExtIdx = path.Length();
		}

		uSize nameIdx = path.FindReverse("/");

		if (nameIdx == 0)
		{
			// @TODO could be optimized to not run twice
			uSize nameIdxBackslash = path.FindReverse("\\");

			if (nameIdxBackslash > nameIdx)
			{
				mNameIdx = nameIdxBackslash + 1;
			}
		}
		else
		{
			mNameIdx = nameIdx + 1;
		}

		mFlags |= FILE_BINARY;
	}

	bool File::Open(FileOpenFlags openFlags)
	{
		bool result = mFuncs.openFunc(*this, openFlags, mpHandle, mFlags);

		if (!result)
		{
			LogError("Failed to load file [%s].", mPath.Str());
			return false;
		}

		mFlags |= FILE_OPEN | FILE_VALID; // Not needed, but just in case

		return true;
	}

	bool File::Close()
	{
		bool result = mFuncs.closeFunc(*this);

		if (!result)
		{
			LogError("Failed to load file [%s].", mPath.Str());
		}

		mpHandle = nullptr;
		mFlags &= ~FILE_OPEN;

		return result;
	}

	bool operator==(const File& file0, const File& file1)
	{
		return file0.mpHandle == file1.mpHandle && file0.mPath == file1.mPath;
	}

	bool operator!=(const File& file0, const File& file1)
	{
		return !operator==(file0, file1);
	}
}