#include "Module/Module.h"

namespace Quartz
{
	void Module::SetLoaded(bool loaded)
	{
		mLoaded = loaded;
	}

	bool Module::Query(bool isEditor, ModuleQueryInfo& queryInfo)
	{
		mQueryResult = mQueryFunc(isEditor, queryInfo);
		return mQueryResult;
	}

	bool Module::Load(Log& engineLog, Engine& engine)
	{
		if(mLoadFunc) return mLoadFunc(engineLog, engine);
		return true;
	}

	void Module::Unload()
	{
		if (mUnloadFunc) mUnloadFunc();
	}

	void Module::PreInit()
	{
		if (mPreInitFunc) mPreInitFunc();
	}

	void Module::Init()
	{
		if (mInitFunc) mInitFunc();
	}

	void Module::PostInit()
	{
		if (mPostInitFunc) mPostInitFunc();
	}

	void Module::Shutdown()
	{
		if (mShutdownFunc) mShutdownFunc();
	}

	Module::Module(
		DynamicLibrary*		pLibrary,
		bool				queryResult,
		ModuleQueryInfo&	queryInfo,
		ModuleQueryFunc		queryFunc,
		ModuleLoadFunc		loadFunc,
		ModuleUnloadFunc	unloadFunc,
		ModulePreInitFunc	preInitFunc,
		ModuleInitFunc		initFunc,
		ModulePostInitFunc	postInitFunc,
		ModuleShutdownFunc	shutdownFunc
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

	bool Module::QuerySuccess() const
	{
		return mQueryResult == true;
	}

	bool Module::IsLoaded() const
	{
		return mLoaded;
	}

	const String& Module::GetName() const
	{
		return mName;
	}

	const String& Module::GetVersion() const
	{
		return mVersion;
	}

	const String& Module::GetPath() const
	{
		return mpLibrary->GetPath();
	}

	DynamicLibrary* Module::GetLibrary()
	{
		return mpLibrary;
	}
}