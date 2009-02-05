/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */
#ifndef _LINUX_HOST_H
#define _LINUX_HOST_H

#include <vector>
#include <string>
#include <api/module.h>
#include <api/module_provider.h>
#include <api/host.h>
#include <api/binding/bound_object.h>

#include <Poco/ScopedLock.h>
#include <Poco/Mutex.h>
#include <Poco/Condition.h>
#include "linux_job.h"

namespace kroll
{

	class EXPORT LinuxHost : public Host
	{ 
	public:
		LinuxHost(int argc, const char* argv[]);

		virtual Module* CreateModule(std::string& path);
		SharedValue InvokeMethodOnMainThread(SharedBoundMethod method,
		                                     const ValueList& args);

		Poco::Mutex& GetJobQueueMutex();
		std::vector<LinuxJob*>& GetJobs();

	protected:
		virtual bool RunLoop();
		virtual ~LinuxHost();

	private:
		Poco::Mutex job_queue_mutex;
		std::vector<LinuxJob*> jobs;
	};
}

extern "C"
{
	EXPORT int Execute(int argc,const char** argv);
}


#endif
