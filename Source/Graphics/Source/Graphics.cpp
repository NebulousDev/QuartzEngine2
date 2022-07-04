#include "System/System.h"
#include "DLL.h"

extern "C"
{
	bool QUARTZENGINE_API SystemQuery(bool isEditor, Quartz::SystemQueryInfo& systemQuery)
	{
		systemQuery.name = "Graphics";
		systemQuery.version = "1.0.0";

		return true;
	}

	bool QUARTZENGINE_API SystemLoad()
	{
		return true;
	}

	void QUARTZENGINE_API SystemUnload()
	{

	}
}