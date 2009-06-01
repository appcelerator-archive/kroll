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
	typedef std::vector<SharedKMethod> EventRecords;

	struct BoundEventEntry
	{
		SharedKMethod method;
		std::string event;
	};

	class APIBinding : public StaticBoundObject
	{
	public:
		APIBinding(Host* host);
		virtual ~APIBinding();

		void Log(int severity, SharedValue);
		int Register(std::string& event, SharedKMethod callback);
		void Unregister(int ref);
		void Fire(const char* event, SharedValue data);

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
		int record;
		Logger* logger;
		std::map<std::string, EventRecords> registrations;
		std::map<int, BoundEventEntry> registrationsById;

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
		void _Register(const ValueList& args, SharedValue result);
		void _Unregister(const ValueList& args, SharedValue result);
		void _Fire(const ValueList& args, SharedValue result);

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

		void _CreateDependency(const ValueList& args, SharedValue value);
		void _InstallDependencies(const ValueList& args, SharedValue value);

		int GetNextRecord();
	};
}

#endif
