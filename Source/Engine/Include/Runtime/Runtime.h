#pragma once

#include "Quartz.h"
#include "Types/Array.h"
#include "Types/Map.h"
#include "Utility/TypeId.h"

#include <functional>

namespace Quartz
{
	template<typename TriggerPayload>
	using RuntimeTriggerFunc = void (*)(const TriggerPayload& payload);

	using RuntimeUpdateFunc = void (*)(double delta);
	using RuntimeTickFunc	= void (*)(uSize tick);

	class QUARTZ_API Runtime
	{
	private:
		static uSize GetTriggerId(const String& triggerName);

		template<typename TriggerPayload>
		class TriggerId
		{
		public:
			static uSize Value()
			{
				static uSize sTriggerId = GetTriggerId(TypeName<TriggerPayload>::Value());
				return sTriggerId;
			}
		};

	private:
		Array<RuntimeUpdateFunc>		mUpdates;
		Array<RuntimeTickFunc>			mTicks;
		Array<Array<void*>>				mTriggers;
		Array<std::function<void()>>	mDefferedTriggers;

		uSize mDirtyUpdateCount		= 0;
		uSize mDirtyTickCount		= 0;
		uSize mDirtyTriggerCount	= 0;

		uSize mTargetUPS;
		uSize mTargetTPS;

		bool mRunning;

	private:
		bool IsValidTriggerId(uSize triggerId);

		void UpdateAll(double delta);
		void TickAll(uSize tick);

		template<typename TriggerPayload>
		void TriggerNow(const TriggerPayload& payload)
		{
			uSize triggerId = TriggerId<TriggerPayload>::Value();

			if (IsValidTriggerId(triggerId))
			{
				for (void* pFunc : mTriggers[triggerId])
				{
					RuntimeTriggerFunc<TriggerPayload> triggerFunc
						= static_cast<RuntimeTriggerFunc<TriggerPayload>>(pFunc);

					if (triggerFunc)
					{
						triggerFunc(payload);
					}
				}

				if (mDirtyTriggerCount > 0)
				{
					Array<void*> cleanTriggers(mTriggers[triggerId].Size() - mDirtyTriggerCount);

					for (void* pFunc : mTriggers[triggerId])
					{
						if (pFunc != nullptr)
						{
							cleanTriggers.PushBack(pFunc);
						}
					}

					Swap(mTriggers[triggerId], cleanTriggers);
					mDirtyTickCount = 0;
				}

			}
		}

	public:
		Runtime();

		void RegisterOnUpdate(RuntimeUpdateFunc updateFunc);
		void RegisterOnTick(RuntimeTickFunc tickFunc);

		template<typename TriggerPayload>
		void RegisterOnTrigger(RuntimeTriggerFunc<TriggerPayload> triggerFunc)
		{
			uSize triggerId = TriggerId<TriggerPayload>::Value();

			if (!IsValidTriggerId(triggerId))
			{
				RegisterTriggerType<TriggerPayload>();
			}

			mTriggers[triggerId].PushBack(static_cast<void*>(triggerFunc));
		}

		void UnregisterOnUpdate(RuntimeUpdateFunc updateFunc);
		void UnregisterOnTick(RuntimeTickFunc tickFunc);

		template<typename TriggerPayload>
		void UnregisterOnTrigger(RuntimeTriggerFunc<TriggerPayload> triggerFunc)
		{
			uSize triggerId = TriggerId<TriggerPayload>::Value();

			if (IsValidTriggerId(triggerId))
			{
				auto& triggers = mTriggers[triggerId];
				auto& triggerIt = triggers.Find(triggerFunc);
				if (triggerIt != triggers.End())
				{
					*triggerIt = nullptr;
					mDirtyTriggerCount++;
				}
			}
		}

		template<typename TriggerPayload>
		void RegisterTriggerType()
		{
			uSize triggerId = TriggerId<TriggerPayload>::Value();

			if (mTriggers.Size() >= triggerId)
			{
				mTriggers.Resize(triggerId + 1);
			}
		}

		template<typename TriggerPayload>
		void Trigger(const TriggerPayload& payload, bool now = false)
		{
			if (now)
			{
				TriggerNow<TriggerPayload>(payload);
			}
			else
			{
				auto& deferedTrigger = [this,payload]() { this->TriggerNow<TriggerPayload>(payload); };
				mDefferedTriggers.PushBack(deferedTrigger);
			}
		}

		void Start();
		void Stop();
	};
}
