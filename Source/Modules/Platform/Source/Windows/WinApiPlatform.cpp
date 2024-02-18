#include "Platform.h"

#include "Windows/WinApi.h"

namespace Quartz
{
	SystemInfo GenerateSystemInfo()
	{
		DWORD dwordOSNameBuffer[1024]{};
		RegGetValueA(
			HKEY_LOCAL_MACHINE, 
			"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion", 
			"ProductName", 
			RRF_RT_ANY,
			NULL, 
			dwordOSNameBuffer, 
			NULL
		);

		String osName = String((const char*)dwordOSNameBuffer);

		SystemInfo sysInfo = {};

		return sysInfo;
	}
}