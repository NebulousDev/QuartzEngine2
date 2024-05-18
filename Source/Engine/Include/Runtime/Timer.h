#pragma once

#include "../EngineAPI.h"
#include <chrono>

namespace Quartz
{
	class QUARTZ_ENGINE_API Timer
	{
	private:
		using ClockType = std::chrono::high_resolution_clock;
		using TimePoint = ClockType::time_point;
		using Duration  = std::chrono::duration<double, std::chrono::nanoseconds::period>;

		TimePoint mStart;
		TimePoint mMark;

	public:
		void	Start();
		double	Mark();
	};
}