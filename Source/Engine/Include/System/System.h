#pragma once

#include "DynamicLibrary.h"
#include "Types/String.h"
#include "Log.h"

namespace Quartz
{
	struct SystemQueryInfo
	{
		String name;
		String version;
	};

	typedef bool (*SystemQueryFunc)(bool isEditor, SystemQueryInfo& systemQuery);
	typedef bool (*SystemLoadFunc)(Log& engineLog);
	typedef void (*SystemUnloadFunc)();

	typedef bool (*SystemPreInitFunc)();
	typedef bool (*SystemInitFunc)();
	typedef bool (*SystemPostInitFunc)();

	typedef bool (*SystemShutdownFunc)();

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
		SystemPreInitFunc	mPreInitFunc;
		SystemInitFunc		mInitFunc;
		SystemPostInitFunc	mPostInitFunc;
		SystemShutdownFunc	mShutdownFunc;

	private:
		void SetLoaded(bool loaded);

		bool Query(bool isEditor, SystemQueryInfo& queryInfo);
		bool Load(Log& engineLog);
		void Unload();
		void PreInit();
		void Init();
		void PostInit();
		void Shutdown();

	public:
		System(
			DynamicLibrary*		pLibrary, 
			bool				queryResult, 
			SystemQueryInfo&	queryInfo,
			SystemQueryFunc		queryFunc,
			SystemLoadFunc		loadFunc,
			SystemUnloadFunc	unloadFunc,
			SystemPreInitFunc	preInitFunc,
			SystemInitFunc		initFunc,
			SystemPostInitFunc	postInitFunc,
			SystemShutdownFunc	shutdownFunc
		);

		bool QuerySuccess() const;
		bool IsLoaded() const;

		const String& GetName() const;
		const String& GetVersion() const;
		const String& GetPath() const;

		DynamicLibrary* GetLibrary();
	};
}