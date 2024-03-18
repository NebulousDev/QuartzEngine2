#pragma once

#include "EngineAPI.h"
#include "Types/Array.h"
#include "Types/Map.h"
#include "Utility/TypeId.h"

#include <functional>

#define RUNTIME_DEFAULT_UPS 350
#define RUNTIME_DEFAULT_TPS 20

namespace Quartz
{
	class Runtime;

	template<typename Event, typename Scope>
	using ScopedRuntimeEventFunc	= void (Scope::*)(Runtime& runtime, const Event& event);
	template<typename Event>
	using RuntimeEventFunc			= void (*)(Runtime& runtime, const Event& event);

	template<typename Scope>
	using ScopedRuntimeUpdateFunc	= void (Scope::*)(Runtime& runtime, double delta);
	using RuntimeUpdateFunc			= void (*)(Runtime& runtime, double delta);

	template<typename Scope>
	using ScopedRuntimeTickFunc		= void (Scope::*)(Runtime& runtime, uSize tick);
	using RuntimeTickFunc			= void (*)(Runtime& runtime, uSize tick);

	using RuntimeID					= uSize;

	class QUARTZ_ENGINE_API Runtime
	{
	public:

		struct UpdateFunctor
		{
			void* pInstance;
			RuntimeUpdateFunc updateFunc;

			virtual void Call(Runtime& runtime, double delta)
			{
				updateFunc(runtime, delta);
			}
		};

		template<typename Scope>
		struct ScopedUpdateFunctor : public UpdateFunctor
		{
			void Call(Runtime& runtime, double delta) override
			{
				ScopedRuntimeUpdateFunc<Scope>* pfunc = (ScopedRuntimeUpdateFunc<Scope>*)reinterpret_cast<void**>(&updateFunc);
				(static_cast<Scope*>(pInstance)->**pfunc)(runtime, delta);
			}
		};

		struct TickFunctor
		{
			void* pInstance;
			RuntimeTickFunc tickFunc;

			virtual void Call(Runtime& runtime, uSize tick)
			{
				tickFunc(runtime, tick);
			}
		};

		template<typename Scope>
		struct ScopedTickFunctor : public TickFunctor
		{
			void Call(Runtime& runtime, uSize tick) override
			{
				ScopedRuntimeTickFunc<Scope>* pfunc = (ScopedRuntimeTickFunc<Scope>*)reinterpret_cast<void**>(&tickFunc);
				(static_cast<Scope*>(pInstance)->**pfunc)(runtime, tick);
			}
		};

		struct EventFunctorBase { virtual void VFT_Filler() {}; };

		template<typename Event>
		struct EventFunctor : public EventFunctorBase
		{
			void* pInstance;
			RuntimeEventFunc<Event> eventFunc;

			virtual void Call(Runtime& runtime, const Event& event)
			{
				eventFunc(runtime, event);
			}
		};

		template<typename Event, typename Scope>
		struct ScopedEventFunctor : public EventFunctor<Event>
		{
			void Call(Runtime& runtime, const Event& event) override
			{
				ScopedRuntimeEventFunc<Event, Scope>* pfunc = 
					(ScopedRuntimeEventFunc<Event, Scope>*)reinterpret_cast<void**>(&eventFunc);
				(static_cast<Scope*>(pInstance)->**pfunc)(runtime, event);
			}
		};

	private:

		Map<String, uSize>	mEventIdMap;
		uSize				mEventIdCount = 0;

		uSize GetEventId(const String& eventName);

		template<typename Event>
		uSize GetEventId()
		{
			static uSize sEventId = GetEventId(TypeName<Event>::Value());
			return sEventId;
		}

		bool IsValidEventId(uSize eventId);

	private:
		Array<UpdateFunctor*>				mUpdates;
		Array<TickFunctor*>					mTicks;
		Array<Array<EventFunctorBase*>>		mEvents;
		Array<std::function<void()>>		mDeferredEvents;

		uSize mDirtyUpdateCount		= 0;
		uSize mDirtyTickCount		= 0;
		uSize mDirtyEventCount		= 0;

		uInt64 mTargetUPS;
		uInt64 mTargetTPS;

		double mCurrentUPS;
		double mCurrentTPS;

		double mUpdateDelta;

		bool mRunning;

	private:
		void UpdateAll(double delta);
		void TickAll(uSize tick);

		void CleanEvents();

		template<typename Event>
		void RegisterEventType()
		{
			uSize eventId = GetEventId<Event>();

			if (mEvents.Size() >= eventId)
			{
				mEvents.Resize(eventId + 1);
			}
		}

		template<typename Event>
		void TriggerNow(const Event& event)
		{
			uSize eventId = GetEventId<Event>();

			if (!IsValidEventId(eventId))
			{
				RegisterEventType<Event>();
			}

			for (EventFunctorBase* pFunctorBase : mEvents[eventId])
			{
				EventFunctor<Event>* pFunctor = 
					static_cast<EventFunctor<Event>*>(*reinterpret_cast<void**>(&pFunctorBase));

				if (pFunctor->eventFunc)
				{
					pFunctor->Call(*this, event);
				}
			}

			if (mDirtyEventCount > 0)
			{
				Array<EventFunctorBase*> cleanTriggers(mEvents[eventId].Size() - mDirtyEventCount);

				for (EventFunctorBase* pFunctorBase : mEvents[eventId])
				{
					EventFunctor<Event>* pFunctor =
						static_cast<EventFunctor<Event>*>(*reinterpret_cast<void**>(&pFunctorBase));

					if (pFunctor->eventFunc)
					{
						cleanTriggers.PushBack(pFunctor);
					}
				}

				Swap(mEvents[eventId], cleanTriggers);
				mDirtyTickCount = 0;
			}
		}

	public:
		Runtime();

		void RegisterOnUpdate(RuntimeUpdateFunc updateFunc);

		template<typename Scope>
		void RegisterOnUpdate(ScopedRuntimeUpdateFunc<Scope> updateFunc, Scope* pInstance)
		{
			ScopedUpdateFunctor<Scope>* pFunctor = new ScopedUpdateFunctor<Scope>();
			pFunctor->pInstance = pInstance;
			pFunctor->updateFunc = static_cast<RuntimeUpdateFunc>(*reinterpret_cast<void**>(&updateFunc));

			mUpdates.PushBack(pFunctor);
		}

		void RegisterOnUpdate(std::function<void()>);

		void RegisterOnTick(RuntimeTickFunc tickFunc);

		template<typename Scope>
		void RegisterOnTick(ScopedRuntimeTickFunc<Scope> tickFunc, Scope* pInstance)
		{
			ScopedTickFunctor<Scope>* pFunctor = new ScopedTickFunctor<Scope>();
			pFunctor->pInstance = pInstance;
			pFunctor->tickFunc = static_cast<RuntimeTickFunc>(*reinterpret_cast<void**>(&tickFunc));

			mTicks.PushBack(pFunctor);
		}

		template<typename Event>
		void RegisterOnEvent(RuntimeEventFunc<Event> eventFunc)
		{
			uSize eventId = GetEventId<Event>();

			if (!IsValidEventId(eventId))
			{
				RegisterEventType<Event>();
			}

			EventFunctor<Event>* pFunctor = new EventFunctor<Event>();
			pFunctor->pInstance = nullptr;
			pFunctor->eventFunc = eventFunc;

			mEvents[eventId].PushBack(pFunctor);
		}

		template<typename Event, typename Scope>
		void RegisterOnEvent(ScopedRuntimeEventFunc<Event, Scope> eventFunc, Scope* pInstance)
		{
			uSize eventId = GetEventId<Event>();

			if (!IsValidEventId(eventId))
			{
				RegisterEventType<Event>();
			}

			ScopedEventFunctor<Event, Scope>* pFunctor = new ScopedEventFunctor<Event, Scope>();
			pFunctor->pInstance = pInstance;
			pFunctor->eventFunc = static_cast<RuntimeEventFunc<Event>>(*reinterpret_cast<void**>(&eventFunc));

			mEvents[eventId].PushBack(pFunctor);
		}

		void UnregisterOnUpdate(RuntimeUpdateFunc updateFunc);

		template<typename Scope>
		void UnregisterOnUpdate(ScopedRuntimeUpdateFunc<Scope> updateFunc, Scope* pInstance)
		{
			for (UpdateFunctor* pUpdateFunctor : mUpdates)
			{
				if (pUpdateFunctor->updateFunc == static_cast<RuntimeUpdateFunc>(*reinterpret_cast<void**>(&updateFunc))
					&& pUpdateFunctor->pInstance == pInstance)
				{
					pUpdateFunctor->updateFunc = nullptr;
					mDirtyUpdateCount++;
				}
			}
		}

		void UnregisterOnTick(RuntimeTickFunc tickFunc);

		template<typename Scope>
		void UnregisterOnTick(ScopedRuntimeTickFunc<Scope> tickFunc, Scope* pInstance)
		{
			for (TickFunctor* pTickFunctor : mTicks)
			{
				if (pTickFunctor->tickFunc == static_cast<RuntimeTickFunc>(*reinterpret_cast<void**>(&tickFunc))
					&& pTickFunctor->pInstance == pInstance)
				{
					pTickFunctor->tickFunc = nullptr;
					mDirtyTickCount++;
				}
			}
		}

		template<typename Event>
		void UnregisterOnEvent(RuntimeEventFunc<Event> eventFunc)
		{
			uSize eventId = GetEventId<Event>();

			if (IsValidEventId(eventId))
			{
				auto& events = mEvents[eventId];
				for (EventFunctorBase* pFunctorBase : events)
				{
					EventFunctor<Event>* pFunctor =
						static_cast<EventFunctor<Event>*>(*reinterpret_cast<void**>(&pFunctorBase));

					if (pFunctor->eventFunc == static_cast<RuntimeEventFunc<Event>>(*reinterpret_cast<void**>(&eventFunc))
						&& pFunctor->pInstance == pInstance)
					{
						pFunctor->eventFunc = nullptr;
						mDirtyEventCount++;
					}
				}
			}
		}

		template<typename Event, typename Scope>
		void UnregisterOnEvent(ScopedRuntimeEventFunc<Event, Scope> eventFunc, Scope* pInstance)
		{
			uSize eventId = GetEventId<Event>();

			if (IsValidEventId(eventId))
			{
				auto& events = mEvents[eventId];
				for(EventFunctorBase * pFunctorBase : events)
				{
					ScopedEventFunctor<Event, Scope>* pFunctor =
						static_cast<ScopedEventFunctor<Event, Scope>*>(*reinterpret_cast<void**>(&pFunctorBase));

					if (pFunctor->eventFunc == static_cast<RuntimeEventFunc<Event>>(*reinterpret_cast<void**>(&eventFunc))
						&& pFunctor->pInstance == pInstance)
					{
						pFunctor->eventFunc = nullptr;
						mDirtyEventCount++;
					}
				}
			}
		}

		template<typename Event>
		void Trigger(const Event& event, bool now = false)
		{
			if (now)
			{
				TriggerNow<Event>(event);
			}
			else
			{
				auto& deferredEvent = [this,event]() { this->TriggerNow<Event>(event); };
				mDeferredEvents.PushBack(deferredEvent);
			}
		}

		void Start();
		void Stop();

		inline void SetTargetUps(uInt64 ups) { mTargetUPS = ups; }
		inline void SetTargetTps(uInt64 tps) { mTargetTPS = tps; }

		inline double GetCurrentUps() const { return mCurrentUPS; }
		inline double GetCurrentTps() const { return mCurrentTPS; }

		inline double GetTargetUps() const { return mTargetUPS; }
		inline double GetTargetTps() const { return mTargetUPS; }

		inline double GetUpdateDelta() const { return mUpdateDelta; }
	};
}
