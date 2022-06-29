#pragma once

#include "System.h"

namespace Quartz
{
	struct System
	{
		SystemQuerryFunc	query;
		SystemOnLoadFunc	onLoad;
		SystemOnUnloadFunc	onUnload;
	};

	class SystemAdmin
	{
	private:

	public:
		System* LoadSystem(const char* dllPath);
		void UnloadSystem(System* pSystem);

		bool QuerySystem(const System* pSystem);

	};
}