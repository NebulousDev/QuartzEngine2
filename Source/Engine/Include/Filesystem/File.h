#pragma once

#include "../EngineAPI.h"
#include "FilesystemHandler.h"
#include "Types/String.h"

namespace Quartz
{
	enum FileFlagBits : uInt16
	{
		FILE_VALID		= 0x001,
		FILE_OPEN		= 0x002,
		FILE_READ		= 0x004,
		FILE_WRITE		= 0x008,
		FILE_BINARY		= 0x010,
		FILE_VIRTUAL	= 0x020,
		FILE_COMPRESSED	= 0x040,
		FILE_HIDDEN		= 0x060,
		FILE_TEMPORARY	= 0x080,
		FILE_IS_FOLDER	= 0x100,
	};

	using FileFlags = uInt16;

	enum FileOpenFlagBits : uSize
	{
		FILE_OPEN_READ		= 0x01,
		FILE_OPEN_WRITE		= 0x02,
		FILE_OPEN_BINARY	= 0x04,
		FILE_OPEN_CREATE	= 0x06,
		FILE_OPEN_HIDDEN	= 0x08,
		FILE_OPEN_TEMPORARY	= 0x10
	};

	using FileOpenFlags = uSize;

	class QUARTZ_ENGINE_API File
	{
	public:
		friend class Filesystem;
		friend class FilesystemHandler;

		using Handler = FilesystemHandler;

	protected:
		String		mPath;
		uInt32		mNameIdx;
		uInt32		mExtIdx;
		uInt32		mSizeBytes;
		uInt32		mOffsetBytes;
		FileFlags	mFlags;

		void*		mpNative;
		Handler*	mpHandler;

	public:
		File();
		File(const String& path, Handler& handler, void* pHandle, FileFlags flags,
			uSize sizeBytes, uSize offsetBytes);

		bool Open(FileOpenFlags openFlags);
		bool Close();

		template<typename ValueType>
		bool Read(ValueType* pOutData, uSize count)
		{
			if (!IsOpen())
			{
				// TODO
				return false;
			}

			return mpHandler->ReadFile(*this, (void*)pOutData, count * sizeof(ValueType));
		}

		template<typename ValueType>
		bool Write(ValueType* pData, uSize count)
		{
			if (!IsOpen())
			{
				// TODO
				return false;
			}

			return mpHandler->WriteFile(*this, (void*)pData, count * sizeof(ValueType));
		}

		friend QUARTZ_ENGINE_API bool operator==(const File& file0, const File& file1);
		friend QUARTZ_ENGINE_API bool operator!=(const File& file0, const File& file1);
		
		const Substring Name() const { return mPath.Substring(mNameIdx); }
		const Substring Extention() const { return mPath.Substring(mExtIdx); }
		const String& Path() const { return mPath; }
		uInt32 Size() const { return mSizeBytes; }
		uInt32 Offset() const { return mOffsetBytes; }
		FileFlags Flags() const { return mFlags; }
		bool IsValid() const { return mFlags & FILE_VALID; }
		bool IsOpen() const { return mFlags & FILE_OPEN; }
		bool IsWritable() const { return mFlags & FILE_WRITE; }
		bool IsReadable() const { return mFlags & FILE_READ; }
		bool IsBinary() const { return mFlags & FILE_BINARY; }
		bool IsVirtual() const { return mFlags & FILE_VIRTUAL; }
		bool IsCompressed() const { return mFlags & FILE_COMPRESSED; }
		bool IsHidden() const { return mFlags & FILE_HIDDEN; }
		bool IsTemporary() const { return mFlags & FILE_TEMPORARY; }
		void* GetNativeHandle() const { return mpNative; }
	};
}