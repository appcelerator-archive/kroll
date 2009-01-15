/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */
#include <iostream>
#include <vector>
#include <cstring>
#include <string>
#if defined(OS_WIN32)
#include <windows.h>
#else
#include <dlfcn.h>
#endif
#include "testhost.h"

namespace kroll
{
	TestHost::TestHost(std::vector<std::string>& modules) : Host(0,0)
	{
	}

	TestHost::~TestHost()
	{
	}

	int TestHost::Run()
	{
		// load our modules through the host implementation but let
		// the base class do the hard work for us
		std::vector<std::string>::iterator iter;
		for (iter = module_paths.begin(); iter != module_paths.end(); iter++)
		{
			FindModules((*iter), modules);
		}

		this->LoadModules(modules);

		return 0;
	}

	Module* TestHost::CreateModule(std::string& path)
	{
		std::cout << "Creating module " << path << std::endl;

	//FIXME - port to Win32
	#if defined(OS_WIN32)
		HMODULE lib_handle = LoadLibrary(path.c_str());
	#else
		void* lib_handle = dlopen(path.c_str(), RTLD_LAZY | RTLD_GLOBAL);
	#endif
		if (!lib_handle)
		{
			std::cerr << "Error load module: " << path << std::endl;
			return 0;
		}

		// get the module factory
	#if defined(OS_WIN32)
		ModuleCreator* create = (ModuleCreator*)GetProcAddress(lib_handle, "CreateModule");
	#else
		ModuleCreator* create = (ModuleCreator*)dlsym(lib_handle, "CreateModule");
	#endif
		if (!create)
		{
			std::cerr << "Error load create entry from module: " << path << std::endl;
			return 0;
		}

		return create(this,path.c_str());
	}
}

