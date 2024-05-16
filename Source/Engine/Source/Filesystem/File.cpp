#include "Filesystem/File.h"

#include "Log.h"

namespace Quartz
{
	File::File() : 
		mPath(), mExtIdx(0), mNameIdx(0), mFlags(FileFlags(0)), 
		mpNative(nullptr), mpMapData(nullptr), mpHandler(nullptr),
		mSizeBytes(0), mOffsetBytes(0), mMapFlags(FileMapFlags(0)) {};

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
			LogError("Failed to open file [%s].", mPath.Str());
			return false;
		}

		return true;
	}

	bool File::Close()
	{
		bool result = mpHandler->CloseFile(*this, mpNative, mFlags);

		if (!result)
		{
			LogError("Failed to close file [%s].", mPath.Str());
		}

		return result;
	}

	bool File::Read(const uInt8* pOutData, uSize sizeBytes)
	{
		if (!IsOpen())
		{
			// TODO
			return false;
		}

		if (!mpHandler->ReadFile(*this, (void*)pOutData, sizeBytes))
		{
			LogError("Error reading from file [%s]. mpHandler->WriteFile() failed.", mPath.Str());
			return false;
		}

		return true;
	}

	bool File::Write(const uInt8* pData, uSize sizeBytes)
	{
		if (!IsOpen())
		{
			// TODO
			return false;
		}

		if (!mpHandler->WriteFile(*this, (void*)pData, sizeBytes))
		{
			LogError("Error writing to file [%s]. mpHandler->WriteFile() failed.", mPath.Str());
			return false;
		}

		return true;
	}

	bool File::Map(uInt8*& pOutMapPtr, uInt64 sizeBytes, FileMapFlags mapFlags)
	{
		// @TODO check perms
		// @TODO check already mapped

		if (!IsOpen())
		{
			LogError("Error memory mapping file [%s]. File is not open.", mPath.Str());
			return false;
		}

		if (!mpHandler->MapFile(*this, pOutMapPtr, sizeBytes, mapFlags))
		{
			LogError("Error memory mapping file [%s]. mpHandler->MapFile() failed.", mPath.Str());
			return false;
		}

		mpMapData = pOutMapPtr;
		mFlags |= FILE_MAPPED;
		mMapFlags = mapFlags;

		return true;
	}

	void File::Unmap()
	{
		mpHandler->UnmapFile(*this);
		mpMapData = nullptr;
		mFlags &= ~FILE_MAPPED;
		mMapFlags = 0;
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