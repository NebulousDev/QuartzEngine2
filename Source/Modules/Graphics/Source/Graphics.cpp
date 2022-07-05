#include "System/System.h"
#include "Quartz.h"

extern "C"
{
	bool QUARTZ_API SystemQuery(bool isEditor, Quartz::SystemQueryInfo& systemQuery)
	{
		systemQuery.name = "GraphicsModule";
		systemQuery.version = "1.0.0";

		return true;
	}

	bool QUARTZ_API SystemLoad()
	{
		return true;
	}

	void QUARTZ_API SystemUnload()
	{

	}
}