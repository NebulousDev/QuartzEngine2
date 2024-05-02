#include "Filesystem/File.h"

#include "Log.h"

namespace Quartz
{
	File::File() : 
		mPath(), mFlags(FileFlags(0)), mpNative(nullptr),
		mSizeBytes(0), mOffsetBytes(0) {};

	File::File(const String& path, Handler& handler, void* pHandle, 
		FileFlags flags, uSize sizeBytes, uSize offsetBytes) :
		mPath(path), mpHandler(&handler), mpNative(pHandle), 
		mFlags(flags), mSizeBytes(sizeBytes), mOffsetBytes(offsetBytes)
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
		bool result = mpHandler->OpenFile(*this, openFlags, mpNative, mFlags);

		if (!result)
		{
			LogError("Failed to load file [%s].", mPath.Str());
			return false;
		}

		return true;
	}

	bool File::Close()
	{
		bool result = mpHandler->CloseFile(*this, mpNative, mFlags);

		if (!result)
		{
			LogError("Failed to load file [%s].", mPath.Str());
		}

		return result;
	}

	bool operator==(const File& file0, const File& file1)
	{
		return file0.mpNative == file1.mpNative && file0.mPath == file1.mPath;
	}

	bool operator!=(const File& file0, const File& file1)
	{
		return !operator==(file0, file1);
	}
}