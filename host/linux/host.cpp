/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license. 
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */
#include <iostream>
#include <vector>
#include <cstring>
#include <dlfcn.h>
#include <string>
#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <api/kroll.h>

#include "host.h"
#include "linux_job.h"

using Poco::ScopedLock;
using Poco::Mutex;

namespace kroll
{
	static gboolean MainThreadJobCallback(gpointer);

	LinuxHost::LinuxHost(int argc, const char *argv[]) : Host(argc, argv)
	{
		gtk_init(&argc, (char***) &argv);

		if (!g_thread_supported())
			g_thread_init(NULL);

		this->mainThread = pthread_self();
	}

	LinuxHost::~LinuxHost()
	{
	}

	void LinuxHost::Exit(int return_code)
	{
		if (this->running)
		{
			// Only call this if gtk main is running.
			// If called when not running will cause an assert!
			gtk_main_quit();
		}
		Host::Exit(return_code);
	}

	const char* LinuxHost::GetPlatform()
	{
		return "linux";
	}

	const char* LinuxHost::GetModuleSuffix()
	{
		return "module.so";
	}

	bool LinuxHost::RunLoop()
	{
		g_timeout_add(250, &MainThreadJobCallback, this);
		gtk_main();
		return false;
	}

	bool LinuxHost::IsMainThread()
	{
		return pthread_equal(this->mainThread, pthread_self());
	}

	Module* LinuxHost::CreateModule(std::string& path)
	{
		void* handle = dlopen(path.c_str(), RTLD_LAZY | RTLD_GLOBAL);
		if (!handle)
		{
			throw ValueException::FromFormat("Error loading module (%s): %s\n", path.c_str(), dlerror());
			return 0;
		}

		// get the module factory
		ModuleCreator* create = (ModuleCreator*) dlsym(handle, "CreateModule");
		if (!create)
		{
			throw ValueException::FromFormat(
				"Cannot load CreateModule symbol from module (%s): %s\n",
				path.c_str(), dlerror());
		}

		std::string dir = FileUtils::GetDirectory(path);
		return create(this, dir.c_str());
	}

	Poco::Mutex& LinuxHost::GetJobQueueMutex()
	{
		return this->jobQueueMutex;
	}

	std::vector<LinuxJob*>& LinuxHost::GetJobs()
	{
		return this->jobs;
	}

	KValueRef LinuxHost::InvokeMethodOnMainThread(
		KMethodRef method,
		const ValueList& args,
		bool synchronous)
	{
		LinuxJob* job = new LinuxJob(method, args, synchronous);
		if (this->IsMainThread() && synchronous)
		{
			job->Execute();
		}
		else
		{
			Poco::ScopedLock<Poco::Mutex> s(this->GetJobQueueMutex());
			this->jobs.push_back(job); // Enqueue job
		}

		if (!synchronous)
		{
			return Value::Undefined; // Handler will cleanup
		}
		else
		{
			// If this is the main thread, Wait() will fall
			// through because we've already called Execute() above.
			job->Wait(); 

			KValueRef r = job->GetResult();
			ValueException e = job->GetException();
			delete job;

			if (!r.isNull())
				return r;
			else
				throw e;
		}
	}

	static gboolean MainThreadJobCallback(gpointer data)
	{
		LinuxHost *host = (LinuxHost*) data;

		// Prevent other threads trying to queue while we clear the queue.
		// But don't block the invocation task while we actually execute
		// the jobs -- one of these jobs may try to add something to the
		// job queue -- deadlock-o-rama
		std::vector<LinuxJob*> jobs;
		{
			Poco::ScopedLock<Poco::Mutex> s(host->GetJobQueueMutex());
			jobs = host->GetJobs();
			host->GetJobs().clear();
		}

		std::vector<LinuxJob*>::iterator j = jobs.begin();
		while (j != jobs.end())
		{
			LinuxJob* job = *j++;

			// Job might be freed soon after Execute()
			bool asynchronous = !job->IsSynchronous();
			job->Execute();

			if (asynchronous)
			{
				job->PrintException();
				delete job;
			}
		}

		return TRUE;
	}
}

extern "C"
{
	int Execute(int argc,const char **argv)
	{
		Host *host = new LinuxHost(argc,argv);
		return host->Run();
	}
}
