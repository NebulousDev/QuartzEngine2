#include "WinApi.h"

#include "Log.h"

namespace Quartz
{
	void WinApiPrintError()
	{
		DWORD error = GetLastError();

		LPSTR pErrorMessage = nullptr;

		LONG length = FormatMessageA(
			FORMAT_MESSAGE_ALLOCATE_BUFFER |
			FORMAT_MESSAGE_FROM_SYSTEM |
			FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL,
			error,
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
			(LPSTR)&pErrorMessage,
			0, NULL);

		if (pErrorMessage == nullptr || length == 0)
		{
			return; // No error
		}

		// Remove newline since FormatMessageA and LogError add one
		pErrorMessage[length - 1] = '\0';

		LogError("WinApi Error: %s", pErrorMessage);

		LocalFree(pErrorMessage);
	}
}
