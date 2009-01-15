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

		std::string p(getenv("KR_PLUGINS"));
		std::string delimiter(":");
		FileUtils::Tokenize(p,module_paths,delimiter);
	}

	LinuxHost::~LinuxHost()
	{
		gtk_main_quit ();
	}

	int LinuxHost::Run()
	{
		std::cout << "Kroll Running (Linux)..." << std::endl;

		std::vector<std::string>::iterator iter = this->module_paths.begin();
		while (iter!=this->module_paths.end())
		{
			this->FindModules((*iter++),this->modules);
		}

		// load our modules through the host implementation but let 
		// the base class do the hard work for us
		this->LoadModules(this->modules);


		//TODO: finish loading
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
}

