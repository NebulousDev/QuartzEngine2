#pragma once

#include "Quartz.h"
#include "Types/Array.h"
#include "Types/Map.h"
#include "Utility/TypeId.h"

#include <functional>

namespace Quartz
{
	class Runtime;

	template<typename Payload, typename Scope>
	using ScopedRuntimeTriggerFunc	= void (Scope::*)(Runtime* pRuntime, const Payload& payload);
	template<typename Payload>
	using RuntimeTriggerFunc		= void (*)(Runtime* pRuntime, const Payload& payload);

	template<typename Scope>
	using ScopedRuntimeUpdateFunc	= void (Scope::*)(Runtime* pRuntime, double delta);
	using RuntimeUpdateFunc			= void (*)(Runtime* pRuntime, double delta);

	template<typename Scope>
	using ScopedRuntimeTickFunc		= void (Scope::*)(Runtime* pRuntime, uSize tick);
	using RuntimeTickFunc			= void (*)(Runtime* pRuntime, uSize tick);

	using RuntimeID					= uSize;

	class QUARTZ_API Runtime
	{
	public:

		struct UpdateFunctor
		{
			void* pInstance;
			RuntimeUpdateFunc updateFunc;

			virtual void Call(Runtime* pRuntime, double delta)
			{
				updateFunc(pRuntime, delta);
			}
		};

		template<typename Scope>
		struct ScopedUpdateFunctor : public UpdateFunctor
		{
			void Call(Runtime* pRuntime, double delta) override
			{
				ScopedRuntimeUpdateFunc<Scope>* pfunc = (ScopedRuntimeUpdateFunc<Scope>*)reinterpret_cast<void**>(&updateFunc);
				(static_cast<Scope*>(pInstance)->**pfunc)(pRuntime, delta);
			}
		};

		struct TickFunctor
		{
			void* pInstance;
			RuntimeTickFunc tickFunc;

			virtual void Call(Runtime* pRuntime, uSize tick)
			{
				tickFunc(pRuntime, tick);
			}
		};

		template<typename Scope>
		struct ScopedTickFunctor : public TickFunctor
		{
			void Call(Runtime* pRuntime, uSize tick) override
			{
				ScopedRuntimeTickFunc<Scope>* pfunc = (ScopedRuntimeTickFunc<Scope>*)reinterpret_cast<void**>(&tickFunc);
				(static_cast<Scope*>(pInstance)->**pfunc)(pRuntime, tick);
			}
		};

		struct TriggerFunctorBase { virtual void VFT_Filler() {}; };

		template<typename Payload>
		struct TriggerFunctor : public TriggerFunctorBase
		{
			void* pInstance;
			RuntimeTriggerFunc<Payload> triggerFunc;

			virtual void Call(Runtime* pRuntime, const Payload& payload)
			{
				triggerFunc(pRuntime, payload);
			}
		};

		template<typename Payload, typename Scope>
		struct ScopedTriggerFunctor : public TriggerFunctor<Payload>
		{
			void Call(Runtime* pRuntime, const Payload& payload) override
			{
				ScopedRuntimeTriggerFunc<Payload, Scope>* pfunc = 
					(ScopedRuntimeTriggerFunc<Payload, Scope>*)reinterpret_cast<void**>(&triggerFunc);
				(static_cast<Scope*>(pInstance)->**pfunc)(pRuntime, payload);
			}
		};

	private:

		Map<String, uSize> mTriggerIdMap;
		uSize mTriggerId = 0;

		uSize GetPayloadId(const String& triggerName);

		template<typename Payload>
		uSize GetPayloadId()
		{
			static uSize sTriggerId = GetPayloadId(TypeName<Payload>::Value());
			return sTriggerId;
		}

	private:
		Array<UpdateFunctor*>				mUpdates;
		Array<TickFunctor*>					mTicks;
		Array<Array<TriggerFunctorBase*>>	mTriggers;
		Array<std::function<void()>>		mDefferedTriggers;

		uSize mDirtyUpdateCount		= 0;
		uSize mDirtyTickCount		= 0;
		uSize mDirtyTriggerCount	= 0;

		uInt64 mTargetUPS;
		uInt64 mTargetTPS;

		bool mRunning;

	private:
		bool IsValidTriggerId(uSize triggerId);

		void UpdateAll(double delta);
		void TickAll(uSize tick);

		void CleanTriggers();

		template<typename TriggerPayload>
		void RegisterTriggerType()
		{
			uSize triggerId = GetPayloadId<TriggerPayload>();

			if (mTriggers.Size() >= triggerId)
			{
				mTriggers.Resize(triggerId + 1);
			}
		}

		template<typename Payload>
		void TriggerNow(const Payload& payload)
		{
			uSize triggerId = GetPayloadId<Payload>();

			if (!IsValidTriggerId(triggerId))
			{
				RegisterTriggerType<Payload>();
			}

			for (TriggerFunctorBase* pFunctorBase : mTriggers[triggerId])
			{
				TriggerFunctor<Payload>* pFunctor = 
					static_cast<TriggerFunctor<Payload>*>(*reinterpret_cast<void**>(&pFunctorBase));

				if (pFunctor->triggerFunc)
				{
					pFunctor->Call(this, payload);
				}
			}

			if (mDirtyTriggerCount > 0)
			{
				Array<TriggerFunctorBase*> cleanTriggers(mTriggers[triggerId].Size() - mDirtyTriggerCount);

				for (TriggerFunctorBase* pFunctorBase : mTriggers[triggerId])
				{
					TriggerFunctor<Payload>* pFunctor =
						static_cast<TriggerFunctor<Payload>*>(*reinterpret_cast<void**>(&pFunctorBase));

					if (pFunctor->triggerFunc)
					{
						cleanTriggers.PushBack(pFunctor);
					}
				}

				Swap(mTriggers[triggerId], cleanTriggers);
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

		void RegisterOnTick(RuntimeTickFunc tickFunc);

		template<typename Scope>
		void RegisterOnTick(ScopedRuntimeTickFunc<Scope> tickFunc, Scope* pInstance)
		{
			ScopedTickFunctor<Scope>* pFunctor = new ScopedTickFunctor<Scope>();
			pFunctor->pInstance = pInstance;
			pFunctor->tickFunc = static_cast<RuntimeTickFunc>(*reinterpret_cast<void**>(&tickFunc));

			mTicks.PushBack(pFunctor);
		}

		template<typename Payload>
		void RegisterOnTrigger(RuntimeTriggerFunc<Payload> triggerFunc)
		{
			uSize payloadId = GetPayloadId<Payload>();

			if (!IsValidTriggerId(payloadId))
			{
				RegisterTriggerType<Payload>();
			}

			TriggerFunctor<Payload> pFunctor = new TriggerFunctor<Payload>();
			pFunctor->pInstance = pInstance;
			pFunctor->tickFunc = triggerFunc;

			mTriggers[payloadId].PushBack(pFunctor);
		}

		template<typename Payload, typename Scope>
		void RegisterOnTrigger(ScopedRuntimeTriggerFunc<Payload, Scope> triggerFunc, Scope* pInstance)
		{
			uSize payloadId = GetPayloadId<Payload>();

			if (!IsValidTriggerId(payloadId))
			{
				RegisterTriggerType<Payload>();
			}

			ScopedTriggerFunctor<Payload, Scope>* pFunctor = new ScopedTriggerFunctor<Payload, Scope>();
			pFunctor->pInstance = pInstance;
			pFunctor->triggerFunc = static_cast<RuntimeTriggerFunc<Payload>>(*reinterpret_cast<void**>(&triggerFunc));

			mTriggers[payloadId].PushBack(pFunctor);
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

		template<typename Payload>
		void UnregisterOnTrigger(RuntimeTriggerFunc<Payload> triggerFunc)
		{
			uSize payloadId = GetPayloadId<Payload>();

			if (IsValidTriggerId(payloadId))
			{
				auto& triggers = mTriggers[payloadId];
				for (TriggerFunctorBase* pFunctorBase : triggers)
				{
					TriggerFunctor<Payload>* pFunctor =
						static_cast<TriggerFunctor<Payload>*>(*reinterpret_cast<void**>(&pFunctorBase));

					if (pFunctor->triggerFunc == static_cast<RuntimeTriggerFunc<Payload>>(*reinterpret_cast<void**>(&triggerFunc))
						&& pFunctor->pInstance == pInstance)
					{
						pFunctor->triggerFunc = nullptr;
						mDirtyTriggerCount++;
					}
				}
			}
		}

		template<typename Payload, typename Scope>
		void UnregisterOnTrigger(ScopedRuntimeTriggerFunc<Payload, Scope> triggerFunc, Scope* pInstance)
		{
			uSize payloadId = GetPayloadId<Payload>();

			if (IsValidTriggerId(payloadId))
			{
				auto& triggers = mTriggers[payloadId];
				for(TriggerFunctorBase * pFunctorBase : triggers)
				{
					ScopedTriggerFunctor<Payload, Scope>* pFunctor =
						static_cast<ScopedTriggerFunctor<Payload, Scope>*>(*reinterpret_cast<void**>(&pFunctorBase));

					if (pFunctor->triggerFunc == static_cast<RuntimeTriggerFunc<Payload>>(*reinterpret_cast<void**>(&triggerFunc))
						&& pFunctor->pInstance == pInstance)
					{
						pFunctor->triggerFunc = nullptr;
						mDirtyTriggerCount++;
					}
				}
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
