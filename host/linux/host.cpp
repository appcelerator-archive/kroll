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

namespace kroll
{
	gboolean main_thread_job_handler(gpointer);

	LinuxHost::LinuxHost(int argc, const char *argv[]) : Host(argc, argv)
	{
		gtk_init(&argc, (char***) &argv);

		char *p = getenv("KR_PLUGINS");
		if (p)
		{
			FileUtils::Tokenize(p, this->module_paths, ":");
		}

	}

	LinuxHost::~LinuxHost()
	{
		gtk_main_quit ();
	}

	int LinuxHost::Run()
	{
		std::cout << "Kroll Running (Linux)..." << std::endl;
		this->AddModuleProvider(this);
		this->LoadModules();

		g_idle_add(&main_thread_job_handler, this);
		gtk_main();

		return 0;
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

	std::vector<Job>& LinuxHost::GetJobs()
	{
		return this->jobs;
	}

	SharedValue LinuxHost::InvokeMethodOnMainThread(SharedBoundMethod method,
	                                                SharedPtr<ValueList> args)
	{
		Poco::ScopedLock<Poco::Mutex> s(this->GetJobQueueMutex());

		Job new_job;
		new_job.method = method;
		new_job.args = args;

		this->jobs.push_back(new_job);
		return Value::Undefined;
	}

	gboolean main_thread_job_handler(gpointer data)
	{
		LinuxHost *host = (LinuxHost*) data;
		Poco::ScopedLock<Poco::Mutex> s(host->GetJobQueueMutex());


		std::vector<Job>& jobs = host->GetJobs();
		if (jobs.size() == 0)
			return TRUE;

		printf("in handler: %i\n", jobs.size());
		std::vector<Job>::iterator i;
		for (i = jobs.begin(); i != jobs.end(); i++)
		{
			SharedBoundMethod method = (*i).method;
			SharedPtr<ValueList> args = (*i).args;
			method->Call(*args);
		}

		jobs.clear();
		printf("end: %i\n", jobs.size());
		return TRUE;
	}
}

kroll::Host* createHost(int argc,const char **argv)
{
	return new LinuxHost(argc,argv);
}

