#pragma once

#include "Quartz.h"
#include "Types/Array.h"

namespace Quartz
{
	typedef void (*RuntimeUpdateFunc)(double delta);

	class QUARTZ_API Runtime
	{
	private:
		Array<RuntimeUpdateFunc> mUpdates;

		bool mRunning;
		uSize mTargetUPS;
		uSize mTargetTPS;

	private:
		void UpdateAll(double delta);

	public:
		Runtime();

		void RegisterUpdate(RuntimeUpdateFunc updateFunc);
		//void RegisterTick();
		//void RegisterTrigger();

		void Start();
		void Stop();
	};
}
