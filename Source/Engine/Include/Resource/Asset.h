#pragma once

#include "EngineAPI.h"
#include "Filesystem/File.h"

namespace Quartz
{
	class QUARTZ_ENGINE_API Asset
	{
	protected:
		File* mpSourceFile;

	public:
		Asset() = default;
		inline Asset(File* pSourceFile) : mpSourceFile(pSourceFile) {}

		inline File* GetSourceFile() { return mpSourceFile; }
	};
}