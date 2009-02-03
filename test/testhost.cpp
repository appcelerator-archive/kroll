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
#include <api/base.h>
#include <windows.h>
#else
#include <dlfcn.h>
#endif
#include "testhost.h"

namespace kroll
{
	TestHost::TestHost(std::vector<std::string> module_paths)
		 : Host(0,0),
		   module_paths(module_paths)
	{

	}

	TestHost::~TestHost()
	{
		ModuleMap::iterator iter = this->modules.begin();
		while (iter != this->modules.end())
		{
			std::string mod_path = iter->first;
			Module *mod = iter->second;
			std::cout << "Trying to unregister: " << mod_path << std::endl;
			this->UnregisterModule(mod);
			iter++;
		}
	}

	void TestHost::TestAll()
	{
		/* Test all modules */
		ModuleMap::iterator iter = this->modules.begin();
		while (iter != this->modules.end())
		{
			std::string mod_path = iter->first;
			Module *mod = iter->second;
			std::cout << "Testing: " << mod_path << std::endl;
			mod->Test();

			iter++;
		}
	}

	Module* TestHost::CreateModule(std::string& path)
	{
		std::cout << "Creating module " << path << std::endl;

	//FIXME - port to Win32
	#if defined(OS_WIN32)
		HMODULE lib_handle = LoadLibraryA(path.c_str());
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

	SharedValue TestHost::InvokeMethodOnMainThread(SharedBoundMethod method,
                                                  SharedPtr<ValueList> args)
	{
		//FIXME - implement for Win32 and Linux. Until then...we
		//will just forward on same thread
		std::cerr << "WARNING: Invoking method on non-main Thread!" << std::endl;
		SharedValue result = method->Call(*args);
		return result;
	}

	bool TestHost::RunLoop()
	{
		return false;
	}

	bool TestHost::Start()
	{
		/* Load all test modules */
		for (size_t i = 0; i < module_paths.size(); i++)
		{
			std::string path = module_paths.at(i);
			ModuleProvider *p = this->FindModuleProvider(path);

			if (p == NULL)
			{
				std::cerr << "Could not find provider for: " << path << std::endl;
				continue;
			}

			bool error = false;
			Module *m = this->LoadModule(path, p);

			if (m == NULL || error)
			{
				std::cerr << "Could not load module: " << path << std::endl;
				continue;
			}

		}

		///* Start all test modules */
		this->StartModules(this->loaded_modules);

		//try {

		SharedPtr<Module> m = this->loaded_modules.at(0);
		this->TestAll();

		return false;
	}
}

