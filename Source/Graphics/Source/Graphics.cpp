
#include "System/System.h"

extern "C"
{
	bool _declspec(dllexport) SystemQuery(bool isEditor, Quartz::SystemQueryInfo& systemQuery)
	{
		systemQuery.name = "Graphics";
		systemQuery.version = "1.0.0";

		return true;
	}

	void _declspec(dllexport) SystemLoad()
	{

	}

	void _declspec(dllexport) SystemUnload()
	{

	}
}