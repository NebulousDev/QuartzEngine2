#include "Runtime/Runtime.h"

#include <chrono>

#include "Log.h"

namespace Quartz
{
	uSize Runtime::GetPayloadId(const String& triggerName)
	{
		auto& idIt = mTriggerIdMap.Find(triggerName);
		if (idIt != mTriggerIdMap.End())
		{
			return idIt->value;
		}

		return mTriggerIdMap.Put(triggerName, mTriggerIdCount++);
	}

	Runtime::Runtime() :
		mRunning(false),
		mTargetTPS(RUNTIME_DEFAULT_TPS),
		mTargetUPS(RUNTIME_DEFAULT_UPS)
	{ }

	bool Runtime::IsValidPayloadId(uSize triggerId)
	{
		return mTriggers.Size() > triggerId;
	}

	void Runtime::RegisterOnUpdate(RuntimeUpdateFunc updateFunc)
	{
		UpdateFunctor* pFunctor = new UpdateFunctor();
		pFunctor->pInstance = nullptr;
		pFunctor->updateFunc = updateFunc;

		mUpdates.PushBack(pFunctor);
	}

	void Runtime::RegisterOnTick(RuntimeTickFunc tickFunc)
	{
		TickFunctor* pFunctor = new TickFunctor();
		pFunctor->pInstance = nullptr;
		pFunctor->tickFunc = tickFunc;

		mTicks.PushBack(pFunctor);
	}

	void Runtime::UnregisterOnUpdate(RuntimeUpdateFunc updateFunc)
	{
		for (UpdateFunctor* pUpdateFunctor : mUpdates)
		{
			if (pUpdateFunctor->updateFunc == updateFunc)
			{
				pUpdateFunctor->updateFunc = nullptr;
				mDirtyUpdateCount++;
			}
		}
	}

	void Runtime::UnregisterOnTick(RuntimeTickFunc tickFunc)
	{
		for (TickFunctor* pTickFunctor : mTicks)
		{
			if (pTickFunctor->tickFunc == tickFunc)
			{
				pTickFunctor->tickFunc = nullptr;
				mDirtyTickCount++;
			}
		}
	}

	void Runtime::UpdateAll(double delta)
	{
		for (UpdateFunctor* pUpdateFunctor : mUpdates)
		{
			if (pUpdateFunctor->updateFunc)
			{
				pUpdateFunctor->Call(this, delta);
			}
		}

		if (mDirtyUpdateCount > 0)
		{
			Array<UpdateFunctor*> cleanUpdates(mUpdates.Size() - mDirtyUpdateCount);
			uSize index = 0;

			for (UpdateFunctor* pUpdateFunctor : mUpdates)
			{
				if (pUpdateFunctor->updateFunc)
				{
					cleanUpdates[index++] = pUpdateFunctor;
				}
				else
				{
					delete pUpdateFunctor;
				}
			}

			Swap(mUpdates, cleanUpdates);
			mDirtyUpdateCount = 0;
		}
	}

	void Runtime::TickAll(uSize tick)
	{
		for (TickFunctor* pTickFunctor : mTicks)
		{
			if (pTickFunctor->tickFunc)
			{
				pTickFunctor->Call(this, tick);
			}
		}

		if (mDirtyTickCount > 0)
		{
			Array<TickFunctor*> cleanTicks(mTicks.Size() - mDirtyTickCount);
			uSize index = 0;

			for (TickFunctor* pTickFunctor : mTicks)
			{
				if (pTickFunctor->tickFunc)
				{
					cleanTicks[index++] = pTickFunctor;
				}
				else
				{
					delete pTickFunctor;
				}
			}

			Swap(mTicks, cleanTicks);
			mDirtyTickCount = 0;
		}
	}

	// Specific type not needed to check null values
	using TriggerType = int;

	void Runtime::CleanTriggers()
	{
		if (mDirtyTriggerCount > 0)
		{
			for (Array<TriggerFunctorBase*>& triggerType : mTriggers)
			{
				Array<TriggerFunctorBase*> cleanTriggerType(mTriggers.Size() - mDirtyTriggerCount);
				uSize index = 0;

				for (TriggerFunctorBase* pFunctorBase : triggerType)
				{
					TriggerFunctor<TriggerType>* pFunctor =
						static_cast<TriggerFunctor<TriggerType>*>(*reinterpret_cast<void**>(&pFunctorBase));

					if (pFunctor->triggerFunc != nullptr)
					{
						cleanTriggerType[index++] = (TriggerFunctorBase*)pFunctor;
					}
					else
					{
						delete pFunctor;
					}
				}

				Swap(triggerType, cleanTriggerType);
			}
		}

		mDirtyTriggerCount = 0;
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

		uInt64 targetTickTime   = SECOND / (mTargetTPS);
		uInt64 targetUpdateTime = SECOND / (mTargetUPS);

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
				mCurrentUPS = accumulatedUpdates;
				mCurrentTPS = accumulatedTicks;

				accumulatedTime = 0;
				accumulatedTicks = 0;
				accumulatedUpdates = 0;
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
				UpdateAll(((double)accumulatedUpdateTime / (double)SECOND));
				accumulatedUpdateTime = 0;
			}

			if (mDefferedTriggers.Size() > 0)
			{
				for (std::function<void()>& defferedTrigger : mDefferedTriggers)
				{
					defferedTrigger();
				}

				mDefferedTriggers.Clear();
			}

			CleanTriggers();
		}
	}

	void Runtime::Stop()
	{
		mRunning = false;
	}
}

