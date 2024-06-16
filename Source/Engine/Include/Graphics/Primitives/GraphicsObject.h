#pragma once

#include "EngineAPI.h"
#include "Types/Types.h"

namespace Quartz
{
	enum GraphicsMemoryLocation
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

	enum GraphicsLoadStatus
	{
		LOAD_STATUS_UNLOADED = 0,
		LOAD_STATUS_LOADING,
		LOAD_STATUS_STREAMING,
		LOAD_STATUS_LOADED
	};

	class QUARTZ_ENGINE_API GraphicsObject
	{
	protected:
		uInt64				mID;
		GraphicsMemoryInfo	mMemoryInfo;
		GraphicsLoadStatus	mLoadStatus;
		void*				mpNativeObject;

	protected:
		void SetLoadStatus(bool isLoaded);

	public:
		inline GraphicsObject() :
			mID(0), mMemoryInfo{}, mLoadStatus(LOAD_STATUS_UNLOADED), mpNativeObject(nullptr) {};

		inline GraphicsObject(const GraphicsMemoryInfo& memoryInfo, void* pNativeObject) :
			mID(0), mMemoryInfo(memoryInfo), mLoadStatus(LOAD_STATUS_UNLOADED), mpNativeObject(pNativeObject) {};

		virtual bool IsValid() const { return mID != 0; }

		inline uInt64 GetID() const { return mID; }

		inline const GraphicsLoadStatus& GetLoadStatus() const { return mLoadStatus; }
		inline const GraphicsMemoryInfo& GetMemoryInfo() const { return mMemoryInfo; }

		inline void* GetNative() const { return mpNativeObject; }
	};
}