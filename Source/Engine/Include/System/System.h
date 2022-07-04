#pragma once

#include "DynamicLibrary.h"
#include "Types/String.h"

namespace Quartz
{
	struct SystemQueryInfo
	{
		String name;
		String version;
	};

	typedef bool (*SystemQueryFunc)(bool isEditor, SystemQueryInfo& systemQuery);
	typedef bool (*SystemLoadFunc)();
	typedef void (*SystemUnloadFunc)();

	class System
	{
	public:
		friend class SystemAdmin;

	private:
		DynamicLibrary*		mpLibrary;
		bool				mQueryResult;
		bool				mLoaded;

		String				mName;
		String				mVersion;

		SystemQueryFunc		mQueryFunc;
		SystemLoadFunc		mLoadFunc;
		SystemUnloadFunc	mUnloadFunc;

	private:
		void SetLoaded(bool loaded);

		bool Query(bool isEditor, SystemQueryInfo& queryInfo);
		bool Load();
		void Unload();

	public:
		System(DynamicLibrary* pLibrary, bool queryResult, SystemQueryInfo& queryInfo,
			SystemQueryFunc queryLoadFunc, SystemLoadFunc loadFunc, SystemUnloadFunc unloadFunc);

		bool QuerySuccess() const;
		bool IsLoaded() const;

		const String& GetName() const;
		const String& GetVersion() const;
		const String& GetPath() const;

		DynamicLibrary* GetLibrary();
	};
}