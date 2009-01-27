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

	SharedValue LinuxHost::InvokeMethodOnMainThread(SharedBoundMethod method,
                                                  SharedPtr<ValueList> args)
	{
		//FIXME - implement for Win32 and Linux. Until then...we
		//will just forward on same thread
		std::cerr << "WARNING: Invoking method on non-main Thread!" << std::endl;
		SharedValue result = method->Call(*args);
		return result;
	}
}

