#pragma once

#include "EngineAPI.h"
#include "Types/Types.h"

namespace Quartz
{
	enum GraphicsMemoryLocation : uInt16
	{
		MEMORY_LOCATION_NONE = 0,
		MEMORY_LOCATION_GPU,
		MEMORY_LOCATION_CPU,
		MEMORY_LOCATION_SHARED
	};

	struct GraphicsMemoryInfo
	{
		GraphicsMemoryLocation	location;
		uInt64					cpuSizeBytes;
		uInt64					gpuSizeBytes;
	};

	enum GraphicsLoadStatus : uInt16
	{
		LOAD_STATUS_UNLOADED = 0,
		LOAD_STATUS_LOADING,
		LOAD_STATUS_STREAMING,
		LOAD_STATUS_LOADED
	};

	enum GraphicsHintFlagBits : uInt16
	{
		GRAPHICS_HINT_NONE			= 0x0 << 0,
		GRAPHICS_HINT_DONT_DESTROY	= 0x1 << 0,
		GRAPHICS_HINT_DONT_UNLOAD	= 0x1 << 1
	};

	using GraphicsHintFlags = GraphicsHintFlagBits;

	class QUARTZ_ENGINE_API GraphicsObject
	{
	protected:
		uInt64				mID;
		GraphicsMemoryInfo	mMemoryInfo;
		GraphicsLoadStatus	mLoadStatus;
		uInt16				mAccessTime;
		GraphicsHintFlags	mHints;
		void*				mpNativeObject;

	protected:
		inline void SetLoadStatus(GraphicsLoadStatus loadStatus) { mLoadStatus = loadStatus; }

	public:
		inline GraphicsObject() :
			mID(0), mMemoryInfo{}, mLoadStatus(LOAD_STATUS_UNLOADED), mpNativeObject(nullptr) {};

		inline GraphicsObject(const GraphicsMemoryInfo& memoryInfo, void* pNativeObject) :
			mID(0), mMemoryInfo(memoryInfo), mLoadStatus(LOAD_STATUS_UNLOADED), mpNativeObject(pNativeObject) {};

		virtual bool IsValid() const { return mID != 0; }

		inline bool IsUnloaded() const { return mLoadStatus == LOAD_STATUS_UNLOADED; }
		inline bool IsLoading() const { return mLoadStatus == LOAD_STATUS_LOADING; }
		inline bool IsLoaded() const { return mLoadStatus == LOAD_STATUS_LOADED; }

		inline void		ResetAccessTime() { mAccessTime = 0; }
		inline void		SetAccessTime(uInt16 time) { mAccessTime = time; }
		inline uInt16	IncrementAccessTime() { return ++mAccessTime; }

		inline uInt64						GetID() const { return mID; }
		inline const GraphicsMemoryInfo&	GetMemoryInfo() const { return mMemoryInfo; }
		inline GraphicsLoadStatus			GetLoadStatus() const { return mLoadStatus; }
		inline uInt16						GetAccessTime() const { return mAccessTime; }
		inline GraphicsHintFlags			GetHints() const { return mHints; }
		inline void*						GetNativeObject() const { return mpNativeObject; }
	};
}