#include "System/System.h"

namespace Quartz
{
	void System::SetLoaded(bool loaded)
	{
		mLoaded = loaded;
	}

	bool System::Query(bool isEditor, SystemQueryInfo& queryInfo)
	{
		mQueryResult = mQueryFunc(isEditor, queryInfo);
		return mQueryResult;
	}

	bool System::Load(Log& engineLog, EntityWorld& entityWorld, Runtime& runtime)
	{
		if(mLoadFunc) return mLoadFunc(engineLog, entityWorld, runtime);
		return true;
	}

	void System::Unload()
	{
		if (mUnloadFunc) mUnloadFunc();
	}

	void System::PreInit()
	{
		if (mPreInitFunc) mPreInitFunc();
	}

	void System::Init()
	{
		if (mInitFunc) mInitFunc();
	}

	void System::PostInit()
	{
		if (mPostInitFunc) mPostInitFunc();
	}

	void System::Shutdown()
	{
		if (mShutdownFunc) mShutdownFunc();
	}

	System::System(
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
	) :
		mpLibrary(pLibrary),
		mQueryResult(queryResult),
		mLoaded(false),
		mName(queryInfo.name),
		mVersion(queryInfo.version),
		mQueryFunc(queryFunc),
		mLoadFunc(loadFunc),
		mUnloadFunc(unloadFunc),
		mPreInitFunc(preInitFunc),
		mInitFunc(initFunc),
		mPostInitFunc(postInitFunc),
		mShutdownFunc(shutdownFunc) { }

	bool System::QuerySuccess() const
	{
		return mQueryResult == true;
	}

	bool System::IsLoaded() const
	{
		return mLoaded;
	}

	const String& System::GetName() const
	{
		return mName;
	}

	const String& System::GetVersion() const
	{
		return mVersion;
	}

	const String& System::GetPath() const
	{
		return mpLibrary->GetPath();
	}

	DynamicLibrary* System::GetLibrary()
	{
		return mpLibrary;
	}
}