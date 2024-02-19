#pragma once

#include <chrono>

namespace Quartz
{
	class Timer
	{
	private:
		using ClockType = std::chrono::steady_clock;
		using TimePoint = ClockType::time_point;
		using Duration  = std::chrono::duration<double, std::chrono::nanoseconds::period>;

		TimePoint mStart;
		TimePoint mMark;

	public:
		void	Start();
		double	Mark();
	};
}