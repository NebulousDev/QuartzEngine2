#pragma once

#include "DynamicLibrary.h"
#include "Entity/World.h"
#include "Runtime/Runtime.h"
#include "Types/String.h"
#include "Log.h"

namespace Quartz
{
	class Engine;

	enum ModuleType
	{
		MODULE_TYPE_GENERAL,
		MODULE_TYPE_GRAPHICS,
		MODULE_TYPE_PLATFORM,
		MODULE_TYPE_GAME
	};

	struct ModuleQueryInfo
	{
		String		name;
		String		version;
		ModuleType	type;
	};

	typedef bool (*ModuleQueryFunc)(bool isEditor, ModuleQueryInfo& outModuleQueryInfo);
	typedef bool (*ModuleLoadFunc)(Log& engineLog, Engine& engine);
	typedef void (*ModuleUnloadFunc)();

	typedef bool (*ModulePreInitFunc)();
	typedef bool (*ModuleInitFunc)();
	typedef bool (*ModulePostInitFunc)();

	typedef bool (*ModuleShutdownFunc)();

	class Module
	{
	public:
		friend class ModuleRegistry;

	private:
		DynamicLibrary*		mpLibrary;
		bool				mQueryResult;
		bool				mLoaded;

		String				mName;
		String				mVersion;
		ModuleType			mType;

		ModuleQueryFunc		mQueryFunc;
		ModuleLoadFunc		mLoadFunc;
		ModuleUnloadFunc	mUnloadFunc;
		ModulePreInitFunc	mPreInitFunc;
		ModuleInitFunc		mInitFunc;
		ModulePostInitFunc	mPostInitFunc;
		ModuleShutdownFunc	mShutdownFunc;

	private:
		void SetLoaded(bool loaded);

		bool Query(bool isEditor, ModuleQueryInfo& queryInfo);
		bool Load(Log& engineLog, Engine& engine);
		void Unload();
		void PreInit();
		void Init();
		void PostInit();
		void Shutdown();

	public:
		Module(
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
		);

		bool				QuerySuccess() const;
		bool				IsLoaded() const;

		const String&		GetName() const;
		const String&		GetVersion() const;
		const String&		GetPath() const;
		const ModuleType	GetType() const;

		DynamicLibrary*		GetLibrary();
	};
}