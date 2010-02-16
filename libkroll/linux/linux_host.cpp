/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license. 
 * Copyright (c) 2008, 2009 Appcelerator, Inc. All Rights Reserved.
 */

#include "../kroll.h"

#include <dlfcn.h>
#include <gcrypt.h>
#include <gdk/gdk.h>
#include <gnutls/gnutls.h>
#include <gtk/gtk.h>
#include <pthread.h>

GCRY_THREAD_OPTION_PTHREAD_IMPL;
using Poco::ScopedLock;
using Poco::Mutex;

namespace kroll
{
	static pthread_t mainThread = 0;

	static gboolean MainThreadJobCallback(gpointer data)
	{
		static_cast<Host*>(data)->RunMainThreadJobs();
		return TRUE;
	}

	void Host::Initialize(int argc, const char *argv[])
	{
		gtk_init(&argc, (char***) &argv);

		if (!g_thread_supported())
			g_thread_init(NULL);

		mainThread = pthread_self();

		// Initialize gnutls for multi-threaded usage.
		gcry_control(GCRYCTL_SET_THREAD_CBS, &gcry_threads_pthread);
		gnutls_global_init();
	}

	Host::~Host()
	{
	}

	void Host::WaitForDebugger()
	{
		printf("Waiting for debugger (Press Any Key to Continue pid=%i)...\n", getpid());
		getchar();
	}

	bool Host::RunLoop()
	{
		string origPath(EnvironmentUtils::Get("KR_ORIG_LD_LIBRARY_PATH"));
		EnvironmentUtils::Set("LD_LIBRARY_PATH", origPath);

		g_timeout_add(250, &MainThreadJobCallback, this);
		gtk_main();
		return false;
	}

	void Host::SignalNewMainThreadJob()
	{
	}

	void Host::ExitImpl(int exitCode)
	{
		// Only call this if gtk_main is running. If called when the gtk_main
		// is not running, it will cause an assertion failure.
		static bool mainLoopRunning = true;
		if (mainLoopRunning)
		{
			mainLoopRunning = false;
			gtk_main_quit();
		}
	}


	Module* Host::CreateModule(std::string& path)
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

		return create(this, FileUtils::GetDirectory(path).c_str());
	}

	bool Host::IsMainThread()
	{
		return pthread_equal(mainThread, pthread_self());
	}
}
