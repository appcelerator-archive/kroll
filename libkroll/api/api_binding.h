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

		void Log(int severity, KValueRef);
		static KListRef ComponentVectorToKList(
			vector<SharedComponent>&,
			KComponentType filter = UNKNOWN);
		static KListRef DependencyVectorToKList(
			std::vector<SharedDependency>&);
		static KListRef ManifestToKList(
			vector<pair<string, string> >&);


		private:
		Host* host;
		KObjectRef global;
		Logger* logger;

		// Use a FastMutex to protect the installer, because we are
		// always trying to lock it in the same thread.
		Poco::FastMutex installerMutex;
		Poco::Thread* installerThread;
		Poco::RunnableAdapter<APIBinding>* installerThreadAdapter;
		vector<SharedDependency> installerDependencies;
		KMethodRef installerCallback;

		void RunInstaller();

		void _Set(const ValueList& args, KValueRef result);
		void _Get(const ValueList& args, KValueRef result);
		void _AddEventListener(const ValueList& args, KValueRef result);
		void _RemoveEventListener(const ValueList& args, KValueRef result);
		void _FireEvent(const ValueList& args, KValueRef result);

		Logger::Level ValueToLevel(KValueRef v);
		void _SetLogLevel(const ValueList& args, KValueRef result);
		void _GetLogLevel(const ValueList& args, KValueRef result);
		void _RunOnMainThread(const ValueList& args, KValueRef result);
		void _RunOnMainThreadAsync(const ValueList& args, KValueRef result);

		void _Print(const ValueList& args, KValueRef result);
		void _Log(const ValueList& args, KValueRef result);
		void _LogTrace(const ValueList& args, KValueRef result);
		void _LogDebug(const ValueList& args, KValueRef result);
		void _LogInfo(const ValueList& args, KValueRef result);
		void _LogNotice(const ValueList& args, KValueRef result);
		void _LogWarn(const ValueList& args, KValueRef result);
		void _LogError(const ValueList& args, KValueRef result);
		void _LogCritical(const ValueList& args, KValueRef result);
		void _LogFatal(const ValueList& args, KValueRef result);

		void _GetInstalledComponentsImpl(KComponentType type, const ValueList& args, KValueRef result);
		void _GetApplication(const ValueList& args, KValueRef value);
		void _GetInstalledComponents(const ValueList& args, KValueRef value);
		void _GetInstalledModules(const ValueList& args, KValueRef value);
		void _GetInstalledSDKs(const ValueList& args, KValueRef value);
		void _GetInstalledMobileSDKs(const ValueList& args, KValueRef value);
		void _GetInstalledRuntimes(const ValueList& args, KValueRef value);
		void _GetComponentSearchPaths(const ValueList& args, KValueRef value);
		void _ReadApplicationManifest(const ValueList& args, KValueRef value);
		void _ComponentGUIDToComponentType(const ValueList& args, KValueRef value);
		void _GetEnvironment(const ValueList& args, KValueRef value);
		
		void _CreateDependency(const ValueList& args, KValueRef value);
		void _InstallDependencies(const ValueList& args, KValueRef value);

		void _CreateKObject(const ValueList& args, KValueRef result);
		void _CreateKMethod(const ValueList& args, KValueRef result);
		void _CreateKList(const ValueList& args, KValueRef result);
		void _CreateBytes(const ValueList& args, KValueRef result);
	};

	/**
	 * An wrapper for a KObject which encapsulates another one for testing
	 */
	class KObjectWrapper : public KObject
	{
	public:
		KObjectWrapper(KObjectRef object);
		void Set(const char *name, KValueRef value);
		KValueRef Get(const char *name);
		bool HasProperty(const char *name);
		SharedStringList GetPropertyNames();
		SharedString DisplayString(int levels);
		bool Equals(KObjectRef other);

	private:
		KObjectRef object;
	};

	/**
	 * An wrapper for a KMethod which encapsulates another one for testing
	 */
	class KMethodWrapper : public KMethod
	{
	public:
		KMethodWrapper(KMethodRef method);
		KValueRef Call(const ValueList& args);
		void Set(const char *name, KValueRef value);
		KValueRef Get(const char *name);
		bool HasProperty(const char *name);
		SharedStringList GetPropertyNames();
		SharedString DisplayString(int levels);
		bool Equals(KObjectRef other);

	private:
		KMethodRef method;
	};

	/**
	 * An wrapper for a KList which encapsulates another one for testing
	 */
	class KListWrapper : public KList
	{
	public:
		KListWrapper(KListRef list);
		void Append(KValueRef value);
		unsigned int Size();
		KValueRef At(unsigned int index);
		void SetAt(unsigned int index, KValueRef value);
		bool Remove(unsigned int index);
		void Set(const char *name, KValueRef value);
		KValueRef Get(const char *name);
		bool HasProperty(const char *name);
		SharedStringList GetPropertyNames();
		SharedString DisplayString(int levels=3);
		bool Equals(KObjectRef other);
	private:
		KListRef list;
	};
}

#endif
