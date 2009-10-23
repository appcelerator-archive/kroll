/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */
#ifndef _LINUX_HOST_H
#define _LINUX_HOST_H

#include <vector>
#include <string>
#include <kroll/kroll.h>

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
		void Exit(int return_code);

		virtual Module* CreateModule(std::string& path);
		KValueRef InvokeMethodOnMainThread(KMethodRef method,
		                                     const ValueList& args,bool waitForCompletion);
		const char* GetPlatform();
		const char* GetModuleSuffix();
		

		Poco::Mutex& GetJobQueueMutex();
		std::vector<LinuxJob*>& GetJobs();
		bool IsMainThread();

	protected:
		virtual bool RunLoop();
		virtual ~LinuxHost();

	private:
		Poco::Mutex jobQueueMutex;
		std::vector<LinuxJob*> jobs;
		pthread_t mainThread;
	};
}

extern "C"
{
	EXPORT int Execute(int argc,const char** argv);
}


#endif
