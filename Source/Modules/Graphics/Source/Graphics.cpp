#include "System/System.h"

#include "Quartz.h"
#include "Entity/World.h"
#include "Runtime/Runtime.h"
#include "Log.h"

using namespace Quartz;

extern "C"
{
	bool QUARTZ_API SystemQuery(bool isEditor, Quartz::SystemQueryInfo& systemQuery)
	{
		systemQuery.name = "GraphicsModule";
		systemQuery.version = "1.0.0";

		return true;
	}

	bool QUARTZ_API SystemLoad(Log& engineLog, EntityWorld& entityWorld, Runtime& runtime)
	{
		//entityWorld.

		return true;
	}

	void QUARTZ_API SystemUnload()
	{

	}
}