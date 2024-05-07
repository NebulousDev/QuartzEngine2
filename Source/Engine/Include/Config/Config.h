#pragma once

#include "../EngineAPI.h"
#include "../Filesystem/File.h"
#include "../Resource/Asset.h"

#include "Types/String.h"
#include "Types/Map.h"
#include "Types/Set.h"

namespace Quartz
{
	class QUARTZ_ENGINE_API Config : public Asset
	{
	public:
		using ConfigHash = uInt64;

	private:
		Map<String, Set<String>>	mCategories;
		Map<String, String>			mConfigs;

	public:
		Config() = default;
		Config(File* pConfigFile);

		// Returns true if the given name is new, false otherwise
		bool SetValue(const String& name, const String& inValue, const String& category = "General");

		// Returns true if the value was found, false otherwise
		bool GetValue(const String& name, String& outValue) const;

		bool Read();
		bool Save() const;

		void SetConfigFile(File* pConfigFile);
		void PrintConfigs();
	};
}