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

	bool System::Load()
	{
		return mLoadFunc();
	}

	void System::Unload()
	{
		mUnloadFunc();
	}

	System::System(DynamicLibrary* pLibrary, bool queryResult, SystemQueryInfo& queryInfo, 
		SystemQueryFunc queryFunc, SystemLoadFunc loadFunc, SystemUnloadFunc unloadFunc) :
		mpLibrary(pLibrary),
		mQueryResult(queryResult),
		mLoaded(false),
		mName(queryInfo.name),
		mVersion(queryInfo.version),
		mQueryFunc(queryFunc),
		mLoadFunc(loadFunc),
		mUnloadFunc(unloadFunc) { }

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