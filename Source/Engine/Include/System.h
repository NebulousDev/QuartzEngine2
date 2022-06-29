#pragma once

namespace Quartz
{
	typedef bool (*SystemQuerryFunc)(bool isEditor);
	typedef void (*SystemOnLoadFunc)();
	typedef void (*SystemOnUnloadFunc)();
}