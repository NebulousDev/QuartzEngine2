#include "Runtime/Runtime.h"

#include <chrono>

#include "Log.h"

namespace Quartz
{
	uSize Runtime::GetTriggerId(const String& triggerName)
	{
		static Map<String, uSize> sTriggerIdMap;
		static uSize sTriggerId = 0;

		auto& idIt = sTriggerIdMap.Find(triggerName);
		if (idIt != sTriggerIdMap.End())
		{
			return idIt->value;
		}

		return sTriggerIdMap.Put(triggerName, sTriggerId++);
	}

	Runtime::Runtime() :
		mRunning(false),
		mTargetTPS(20),
		mTargetUPS(300)
	{ }

	bool Runtime::IsValidTriggerId(uSize triggerId)
	{
		return mTriggers.Size() >= triggerId;
	}

	void Runtime::RegisterOnUpdate(RuntimeUpdateFunc updateFunc)
	{
		mUpdates.PushBack(updateFunc);
	}

	void Runtime::RegisterOnTick(RuntimeTickFunc tickFunc)
	{
		mTicks.PushBack(tickFunc);
	}

	void Runtime::UnregisterOnUpdate(RuntimeUpdateFunc updateFunc)
	{
		auto& updateIt = mUpdates.Find(updateFunc);
		if (updateIt != mUpdates.End())
		{
			*updateIt = nullptr;
			mDirtyUpdateCount++;
		}
	}

	void Runtime::UnregisterOnTick(RuntimeTickFunc tickFunc)
	{
		auto& tickIt = mTicks.Find(tickFunc);
		if (tickIt != mTicks.End())
		{
			*tickIt = nullptr;
			mDirtyTickCount++;
		}
	}

	void Runtime::UpdateAll(double delta)
	{
		for (RuntimeUpdateFunc& updateFunc : mUpdates)
		{
			if (updateFunc)
			{
				updateFunc(delta);
			}
		}

		if (mDirtyUpdateCount > 0)
		{
			Array<RuntimeUpdateFunc> cleanUpdates(mUpdates.Size() - mDirtyUpdateCount);

			for (RuntimeUpdateFunc pFunc : mUpdates)
			{
				if (pFunc != nullptr)
				{
					cleanUpdates.PushBack(pFunc);
				}
			}

			Swap(mUpdates, cleanUpdates);
			mDirtyUpdateCount = 0;
		}
	}

	void Runtime::TickAll(uSize tick)
	{
		for (RuntimeTickFunc& tickFunc : mTicks)
		{
			if (tickFunc)
			{
				tickFunc(tick);
			}
		}

		if (mDirtyTickCount > 0)
		{
			Array<RuntimeTickFunc> cleanTicks(mTicks.Size() - mDirtyTickCount);

			for (RuntimeTickFunc pFunc : mTicks)
			{
				if (pFunc != nullptr)
				{
					cleanTicks.PushBack(pFunc);
				}
			}

			Swap(mTicks, cleanTicks);
			mDirtyTickCount = 0;
		}
	}

	uInt64 GetTimeNanoseconds()
	{
		auto& now = std::chrono::high_resolution_clock::now().time_since_epoch();
		return std::chrono::duration_cast<std::chrono::nanoseconds>(now).count();
	}

	void Runtime::Start()
	{
		// Dont run more than one runtime
		if (mRunning) return; 
		mRunning = true;

		constexpr uInt64 SECOND = 1000000000ll;
		
		uInt64 currentTime				= 0;
		uInt64 lastTime					= 0;
		uInt64 deltaTime				= 0;
		uInt64 accumulatedTime			= 0;
		uInt64 accumulatedTickTime		= 0;
		uInt64 accumulatedUpdateTime	= 0;
		uInt64 accumulatedUpdates		= 0;
		uInt64 accumulatedTicks			= 0;

		uInt64 targetTickTime   = SECOND / (mTargetTPS + 1);
		uInt64 targetUpdateTime = SECOND / (mTargetUPS + 1);

		currentTime = GetTimeNanoseconds();
		lastTime = currentTime;

		while (mRunning)
		{
			currentTime			= GetTimeNanoseconds();
			deltaTime			= currentTime - lastTime;
			lastTime			= currentTime;

			accumulatedTime			+= deltaTime;
			accumulatedUpdateTime	+= deltaTime;
			accumulatedTickTime		+= deltaTime;

			if (accumulatedTime >= SECOND)
			{
				accumulatedTickTime = 0;
				accumulatedUpdateTime = 0;
				accumulatedTime = 0;

				LogInfo("UPS: %d, TPS: %d", accumulatedUpdates, accumulatedTicks);

				accumulatedUpdates = 0;
				accumulatedTicks = 0;
			}

			if (accumulatedTickTime >= targetTickTime)
			{
				accumulatedTicks++;
				TickAll(accumulatedTicks - 1);
				accumulatedTickTime = 0;
			}

			if (accumulatedUpdateTime >= targetUpdateTime)
			{
				accumulatedUpdates++;
				UpdateAll(deltaTime / SECOND);
				accumulatedUpdateTime = 0;
			}
		}
	}

	void Runtime::Stop()
	{
		mRunning = false;
	}
}

