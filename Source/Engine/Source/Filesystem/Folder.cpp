#include "Filesystem/Folder.h"

namespace Quartz
{
	bool operator==(const Folder& folder0, const Folder& folder1)
	{
		return folder0.name == folder1.name;
	}
}