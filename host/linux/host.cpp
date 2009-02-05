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

		char *p = getenv("KR_MODULES");
		if (p)
		{
			FileUtils::Tokenize(p, this->module_paths, ":");
		}

		std::cout << "Kroll Running (Linux)..." << std::endl;
	}

	LinuxHost::~LinuxHost()
	{
		gtk_main_quit();
	}

	bool LinuxHost::RunLoop()
	{
		g_idle_add(&main_thread_job_handler, this);
		gtk_main();
		return false;
	}

	Module* LinuxHost::CreateModule(std::string& path)
	{
		std::cout << "Creating module " << path << std::endl;


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
		return create(this,FileUtils::GetDirectory(path));
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
		SharedBoundMethod method,
		const ValueList& args)
	{
		printf("invoking on main method\n");
		LinuxJob* job = new LinuxJob(method, args);

		{
			Poco::ScopedLock<Poco::Mutex> s(this->GetJobQueueMutex());
			this->jobs.push_back(job); // Enqueue job
		}
		job->Wait(); // Wait for processing

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
		std::vector<LinuxJob*>::iterator j;

		if (jobs.size() == 0)
			return TRUE;

		for (j = jobs.begin(); j != jobs.end(); j++)
		{
			(*j)->Execute();
		}

		jobs.clear();
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
