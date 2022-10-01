#include "Runtime/Runtime.h"

#include <chrono>

#include "Log.h"

namespace Quartz
{
	Runtime::Runtime() :
		mTargetTPS(20),
		mTargetUPS(300)
	{ }

	void Runtime::RegisterUpdate(RuntimeUpdateFunc updateFunc)
	{
		mUpdates.PushBack(updateFunc);
	}

	void Runtime::UpdateAll(double delta)
	{
		for (RuntimeUpdateFunc& updateFunc : mUpdates)
		{
			updateFunc(delta);
		}
	}

	uInt64 GetTimeNanoseconds()
	{
		auto& now = std::chrono::high_resolution_clock::now().time_since_epoch();
		return std::chrono::duration_cast<std::chrono::nanoseconds>(now).count();
	}

	void Runtime::Start()
	{
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
				//Tick(accumulatedTicks);
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

