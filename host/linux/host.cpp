/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license. 
 * Copyright (c) 2008, 2009 Appcelerator, Inc. All Rights Reserved.
 */

#include "host.h"

#include <iostream>
#include <vector>
#include <cstring>
#include <dlfcn.h>
#include <string>
#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <api/kroll.h>

#include <gnutls/gnutls.h>
#include <gcrypt.h>
#include <errno.h>
#include <pthread.h>

GCRY_THREAD_OPTION_PTHREAD_IMPL;
using Poco::ScopedLock;
using Poco::Mutex;

namespace kroll
{
	static gboolean MainThreadJobCallback(gpointer data)
	{
		static_cast<Host*>(data)->RunMainThreadJobs();
		return TRUE;
	}

	LinuxHost::LinuxHost(int argc, const char *argv[]) : Host(argc, argv)
	{
		gtk_init(&argc, (char***) &argv);

		if (!g_thread_supported())
			g_thread_init(NULL);

		this->mainThread = pthread_self();

		// Initialize gnutls for multi-threaded usage.
		gcry_control(GCRYCTL_SET_THREAD_CBS, &gcry_threads_pthread);
		gnutls_global_init();
	}

	LinuxHost::~LinuxHost()
	{
	}

	void LinuxHost::Exit(int returnCode)
	{
		// Only call this if gtk_main is running. If called when the gtk_main
		// is not running, it will cause an assertion failure.
		static bool mainLoopRunning = true;
		if (mainLoopRunning)
		{
			mainLoopRunning = false;
			gtk_main_quit();
		}

		Host::Exit(returnCode);
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
		string origPath(EnvironmentUtils::Get("KR_ORIG_LD_LIBRARY_PATH"));
		EnvironmentUtils::Set("LD_LIBRARY_PATH", origPath);

		g_timeout_add(250, &MainThreadJobCallback, this);
		gtk_main();
		return false;
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

	bool LinuxHost::IsMainThread()
	{
		return pthread_equal(this->mainThread, pthread_self());
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
