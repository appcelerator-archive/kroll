/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */
#ifndef _WIN32_HOST_H
#define _WIN32_HOST_H

#include <api/base.h>
#include <api/kroll.h>
#include <string>
#include <vector>
#include <windows.h>
#include <Poco/ScopedLock.h>
#include <Poco/Mutex.h>
#include <Poco/Condition.h>
#include "win32_job.h"
 
#ifdef KROLL_HOST_EXPORT
# define KROLL_HOST_API __declspec(dllexport)
#else
# define KROLL_HOST_API __declspec(dllimport)
#endif

namespace kroll
{
	class KROLL_HOST_API Win32Host : public Host
	{
	public:
		static void InitOLE();

		Win32Host(HINSTANCE hInstance, int argc, const char **argv);
		virtual ~Win32Host();

		virtual Module* CreateModule(std::string& path);

		HINSTANCE GetInstanceHandle() { return instance_handle; }
		SharedValue InvokeMethodOnMainThread(SharedKMethod method,
		                                     const ValueList& args,
											 bool waitForCompletion=true);
		const char* GetPlatform();
		const char* GetModuleSuffix();

	protected:
		bool RunLoop();
		bool Start();
		Poco::Mutex& GetJobQueueMutex();
		std::vector<Win32Job*>& GetJobs();

	private:
		HINSTANCE instance_handle;
		static bool ole_initialized;
		void InvokeMethods();
		DWORD thread_id;
		Poco::Mutex job_queue_mutex;
		std::vector<Win32Job*> jobs;
	};
}

extern "C"
{
	KROLL_HOST_API int Execute(HINSTANCE hInstance, int argc, const char **argv);
}

#endif
