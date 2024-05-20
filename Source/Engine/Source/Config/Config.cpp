#include "Config/Config.h"

#include "assert.h"
#include "Log.h"
#include "Utility/StringReader.h"

namespace Quartz
{
	Config::Config(File* pConfigFile) : Asset(pConfigFile)
	{
		assert(mpSourceFile->IsValid());

		if (!mpSourceFile->IsOpen())
		{
			mpSourceFile->Open(FILE_OPEN_READ | FILE_OPEN_WRITE);
		}
	}

	bool Config::SetValue(const String& name, const String& inValue, const String& category)
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

	bool Config::GetValue(const String& name, String& outValue) const
	{
		auto& configIt = mConfigs.Find(name);
		if (configIt != mConfigs.End())
		{
			outValue = configIt->value;
			return true;
		}

		return false;
	}

	bool Config::Read()
	{
		const uSize fileSizeBytes = mpSourceFile->GetSize();
		char* pFileData = new char[fileSizeBytes + 1];
		pFileData[fileSizeBytes] = '\0';
		
		if (!mpSourceFile->Read((uInt8*)pFileData, fileSizeBytes))
		{
			LogError("Failed to read config file [%s]!", mpSourceFile->GetPath());
			return false;
		}

		StringReader reader(String(pFileData, fileSizeBytes));
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
						mpSourceFile->GetPath(), lineNumber);

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
						mpSourceFile->GetPath(), lineNumber);

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

	bool Config::Save() const
	{
		return false;
	}

	void Config::SetConfigFile(File* pConfigFile)
	{
		assert(pConfigFile);
		assert(pConfigFile->IsValid());
		assert(pConfigFile->GetExtention() == "ini"_STR);

		mpSourceFile = pConfigFile;

		if (!mpSourceFile->IsOpen())
		{
			mpSourceFile->Open(FILE_OPEN_READ | FILE_OPEN_WRITE);
		}
	}

	void Config::PrintConfigs()
	{
		LogInfo("Config file [%s]:", mpSourceFile->GetPath().Str());

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