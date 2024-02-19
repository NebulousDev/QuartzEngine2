#include "Runtime/Timer.h"

namespace Quartz
{
	void Timer::Start()
	{
		mStart = std::chrono::steady_clock::now();
		mMark = mStart;
	}

	double Timer::Mark()
	{
		auto& now = std::chrono::steady_clock::now();
		auto& time = std::chrono::duration_cast<Duration>(now - mMark);
		mMark = now;

		return time.count();
	}
}