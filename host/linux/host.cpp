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
	gboolean main_thread_job_handler(gpointer);

	LinuxHost::LinuxHost(int argc, const char *argv[]) : Host(argc, argv)
	{
		gtk_init(&argc, (char***) &argv);
		this->main_thread = pthread_self();
	}

	LinuxHost::~LinuxHost()
	{
	}

	void LinuxHost::Exit(int return_code)
	{
		Host::Exit(return_code);
		gtk_main_quit();
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
		g_timeout_add(250, &main_thread_job_handler, this);
		gtk_main();
		return false;
	}

	bool LinuxHost::IsMainThread()
	{
		return pthread_equal(this->main_thread, pthread_self());
	}

	Module* LinuxHost::CreateModule(std::string& path)
	{
		void* lib_handle = dlopen(path.c_str(), RTLD_LAZY | RTLD_GLOBAL);
		if (!lib_handle)
		{
			std::cerr << "Error load module: " << path << std::endl;
			std::cerr << "Error: " << dlerror()  << std::endl;
			return 0;
		}

		// get the module factory
		ModuleCreator* create = (ModuleCreator*)dlsym(lib_handle, "CreateModule");
		if (!create)
		{
			std::cerr << "Error load create entry from module: " << path << std::endl;
			return 0;
		}

		std::string dir = FileUtils::GetDirectory(path);
		return create(this, dir.c_str());
	}

	Poco::Mutex& LinuxHost::GetJobQueueMutex()
	{
		return this->job_queue_mutex;
	}

	std::vector<LinuxJob*>& LinuxHost::GetJobs()
	{
		return this->jobs;
	}

	SharedValue LinuxHost::InvokeMethodOnMainThread(
		SharedKMethod method,
		const ValueList& args,
		bool waitForCompletion)
	{
		LinuxJob* job = new LinuxJob(method, args, waitForCompletion);

		if (this->IsMainThread())
		{
			job->Execute();
		}
		else
		{
			Poco::ScopedLock<Poco::Mutex> s(this->GetJobQueueMutex());
			this->jobs.push_back(job); // Enqueue job
		}

		if (!waitForCompletion)
			return Value::Undefined; // Handler will cleanup

		// If this isn't the main thread we 
		// need to wait for our job to finish
		if (!this->IsMainThread())
			job->Wait(); 

		SharedValue r = job->GetResult();
		ValueException e = job->GetException();
		delete job;

		if (!r.isNull())
			return r;
		else
			throw e;
	}

	gboolean main_thread_job_handler(gpointer data)
	{
		LinuxHost *host = (LinuxHost*) data;
		// Prevent other threads trying to queue jobs.
		Poco::ScopedLock<Poco::Mutex> s(host->GetJobQueueMutex());

		std::vector<LinuxJob*>& jobs = host->GetJobs();
		if (jobs.size() == 0)
			return TRUE;

		std::vector<LinuxJob*>::iterator j = jobs.begin();
		while (j != jobs.end())
		{
			LinuxJob* job = *j;
			j = jobs.erase(j);

			job->Execute();
			if (!job->IsWaitingForCompletion())
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
