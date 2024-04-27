#include "Config/Config.h"

#include "assert.h"
#include "Log.h"
#include "Utility/StringReader.h"

namespace Quartz
{
	EngineConfig::EngineConfig(File* pConfigFile) : mpConfigFile(pConfigFile)
	{
		assert(pConfigFile->IsValid());

		if (!mpConfigFile->IsOpen())
		{
			mpConfigFile->Open(FILE_OPEN_READ | FILE_OPEN_WRITE);
		}
	}

	bool EngineConfig::SetValue(const String& name, const String& inValue, const String& category)
	{
		bool found = false;

		auto& configIt = mConfigs.Find(name);
		if (configIt != mConfigs.End())
		{
			configIt->value = inValue;
			found = true;
		}
		else
		{
			mConfigs.Put(name, inValue);
		}

		auto& categoryIt = mCategories.Find(category);
		if (categoryIt != mCategories.End())
		{
			categoryIt->value.Add(name);
		}
		else
		{
			Set<String>& configs = mCategories.Put(category, Set<String>());
			configs.Add(name);
		}

		return found;
	}

	bool EngineConfig::GetValue(const String& name, String& outValue) const
	{
		auto& configIt = mConfigs.Find(name);
		if (configIt != mConfigs.End())
		{
			outValue = configIt->value;
			return true;
		}

		return false;
	}

	bool EngineConfig::Read()
	{
		const uSize fileSizeBytes = mpConfigFile->Size();
		const char* pFileData = new char[fileSizeBytes];
		
		if (!mpConfigFile->Read(pFileData, fileSizeBytes))
		{
			LogError("Failed to read config file [%s]!", mpConfigFile->Path());
			return false;
		}

		StringReader<String> reader(String(pFileData, fileSizeBytes));
		uSize lineNumber = 0;
		Substring category;

		while (!reader.IsEmpty())
		{
			Substring line = reader.ReadLine();
			line = line.TrimWhitespace();

			lineNumber++;

			if (line.IsEmpty())
			{
				continue;
			}
			else if (line.Str()[0] == '[')
			{
				uSize endIdx = line.Find("]");

				if (endIdx == line.Length())
				{
					LogError("Error loading Config. File [%s] is missing a closing bracket ']' on line %d.", 
						mpConfigFile->Path(), lineNumber);

					//mConfigs.Clear();
					//mCategories.Clear();
					continue;
				}

				category = line.Substring(1, endIdx).TrimWhitespace();

				if (mCategories.Find(String(category)) == mCategories.End())
				{
					mCategories.Put(category, Set<String>());
				}
			}
			else if (line.Str()[0] == ';')
			{
				continue; // Comments
			}
			else
			{
				uSize assignIdx = line.Find("=");

				if (assignIdx == line.Length())
				{
					LogError("Error loading Config. File [%s] is missing an assignment '=' on line %d.",
						mpConfigFile->Path(), lineNumber);

					//mConfigs.Clear();
					//mCategories.Clear();
					continue;
				}

				Substring keyStr	= line.Substring(0, assignIdx).TrimWhitespace();
				Substring valueStr	= line.Substring(assignIdx + 1).TrimWhitespace();

				SetValue(keyStr, valueStr, category);
			}
		}

		delete[] pFileData;

		return true;
	}

	bool EngineConfig::Save() const
	{
		return false;
	}

	void EngineConfig::SetConfigFile(File* pConfigFile)
	{
		assert(pConfigFile);
		assert(pConfigFile->IsValid());
		assert(pConfigFile->Extention() == "ini"_STR);

		mpConfigFile = pConfigFile;

		if (!mpConfigFile->IsOpen())
		{
			mpConfigFile->Open(FILE_OPEN_READ | FILE_OPEN_WRITE);
		}
	}

	void EngineConfig::PrintConfigs()
	{
		LogInfo("Config file [%s]:", mpConfigFile->Path().Str());

		for (auto& categoryIt : mCategories)
		{
			LogRaw("  [%s]\n", categoryIt.key.Str());

			for (auto& valueIt : categoryIt.value)
			{
				String value;
				GetValue(valueIt, value);
				LogRaw("    %s = %s\n", valueIt.Str(), value.Str());
			}
		}
	}
}