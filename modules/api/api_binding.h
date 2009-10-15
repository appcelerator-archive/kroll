/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */

#ifndef _API_BINDING_H_
#define _API_BINDING_H_

#include <kroll/kroll.h>
#include <map>
#include <vector>
#include <string>
#include <Poco/Thread.h>
#include <Poco/RunnableAdapter.h>

namespace kroll
{
	class APIBinding : public KAccessorObject
	{
		public:
		APIBinding(Host* host);
		virtual ~APIBinding();

		void Log(int severity, SharedValue);
		static SharedKList ComponentVectorToKList(
			vector<SharedComponent>&,
			KComponentType filter = UNKNOWN);
		static SharedKList DependencyVectorToKList(
			std::vector<SharedDependency>&);
		static SharedKList ManifestToKList(
			vector<pair<string, string> >&);


		private:
		Host* host;
		SharedKObject global;
		Logger* logger;

		// Use a FastMutex to protect the installer, because we are
		// always trying to lock it in the same thread.
		Poco::FastMutex installerMutex;
		Poco::Thread* installerThread;
		Poco::RunnableAdapter<APIBinding>* installerThreadAdapter;
		vector<SharedDependency> installerDependencies;
		SharedKMethod installerCallback;

		void RunInstaller();

		void _Set(const ValueList& args, SharedValue result);
		void _Get(const ValueList& args, SharedValue result);
		void _AddEventListener(const ValueList& args, SharedValue result);
		void _RemoveEventListener(const ValueList& args, SharedValue result);
		void _FireEvent(const ValueList& args, SharedValue result);

		Logger::Level ValueToLevel(SharedValue v);
		void _SetLogLevel(const ValueList& args, SharedValue result);
		void _GetLogLevel(const ValueList& args, SharedValue result);
		void _RunOnMainThread(const ValueList& args, SharedValue result);
		void _RunOnMainThreadAsync(const ValueList& args, SharedValue result);

		void _Print(const ValueList& args, SharedValue result);
		void _Log(const ValueList& args, SharedValue result);
		void _LogTrace(const ValueList& args, SharedValue result);
		void _LogDebug(const ValueList& args, SharedValue result);
		void _LogInfo(const ValueList& args, SharedValue result);
		void _LogNotice(const ValueList& args, SharedValue result);
		void _LogWarn(const ValueList& args, SharedValue result);
		void _LogError(const ValueList& args, SharedValue result);
		void _LogCritical(const ValueList& args, SharedValue result);
		void _LogFatal(const ValueList& args, SharedValue result);

		void _GetInstalledComponentsImpl(KComponentType type, const ValueList& args, SharedValue result);
		void _GetApplication(const ValueList& args, SharedValue value);
		void _GetInstalledComponents(const ValueList& args, SharedValue value);
		void _GetInstalledModules(const ValueList& args, SharedValue value);
		void _GetInstalledSDKs(const ValueList& args, SharedValue value);
		void _GetInstalledMobileSDKs(const ValueList& args, SharedValue value);
		void _GetInstalledRuntimes(const ValueList& args, SharedValue value);
		void _GetComponentSearchPaths(const ValueList& args, SharedValue value);
		void _ReadApplicationManifest(const ValueList& args, SharedValue value);
		void _ComponentGUIDToComponentType(const ValueList& args, SharedValue value);
		void _GetEnvironment(const ValueList& args, SharedValue value);
		
		void _CreateDependency(const ValueList& args, SharedValue value);
		void _InstallDependencies(const ValueList& args, SharedValue value);

		void _CreateKObject(const ValueList& args, SharedValue result);
		void _CreateKMethod(const ValueList& args, SharedValue result);
		void _CreateKList(const ValueList& args, SharedValue result);
		void _CreateBlob(const ValueList& args, SharedValue result);
	};

	/**
	 * An wrapper for a KObject which encapsulates another one for testing
	 */
	class KObjectWrapper : public KObject
	{
	public:
		KObjectWrapper(SharedKObject object);
		void Set(const char *name, SharedValue value);
		SharedValue Get(const char *name);
		bool HasProperty(const char *name);
		SharedStringList GetPropertyNames();
		SharedString DisplayString(int levels);
		bool Equals(SharedKObject other);

	private:
		SharedKObject object;
	};

	/**
	 * An wrapper for a KMethod which encapsulates another one for testing
	 */
	class KMethodWrapper : public KMethod
	{
	public:
		KMethodWrapper(SharedKMethod method);
		SharedValue Call(const ValueList& args);
		void Set(const char *name, SharedValue value);
		SharedValue Get(const char *name);
		bool HasProperty(const char *name);
		SharedStringList GetPropertyNames();
		SharedString DisplayString(int levels);
		bool Equals(SharedKObject other);

	private:
		SharedKMethod method;
	};

	/**
	 * An wrapper for a KList which encapsulates another one for testing
	 */
	class KListWrapper : public KList
	{
	public:
		KListWrapper(SharedKList list);
		void Append(SharedValue value);
		unsigned int Size();
		SharedValue At(unsigned int index);
		void SetAt(unsigned int index, SharedValue value);
		bool Remove(unsigned int index);
		void Set(const char *name, SharedValue value);
		SharedValue Get(const char *name);
		bool HasProperty(const char *name);
		SharedStringList GetPropertyNames();
		SharedString DisplayString(int levels=3);
		bool Equals(SharedKObject other);
	private:
		SharedKList list;
	};
}

#endif
