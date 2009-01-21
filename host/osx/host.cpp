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
#import <Cocoa/Cocoa.h>
#include "host.h"
#include "log.h"

namespace kroll
{
	OSXHost::OSXHost(int _argc, const char **_argv) : Host(_argc,_argv)
	{
		SetupLog(_argc,_argv,[NSString stringWithFormat:@"%s/run.log",this->GetApplicationHome().c_str()]);

		char *p = getenv("KR_PLUGINS");
		if (p)
		{
			FileUtils::Tokenize(p, this->module_paths, ":");
		}
	}

	OSXHost::~OSXHost()
	{
		CloseLog();
	}

	int OSXHost::Run()
	{
		TRACE(@PRODUCT_NAME" Kroll Running (OSX)...");
		this->AddModuleProvider(this);
		this->LoadModules();

		[NSApp run];
		return 0;
	}

	Module* OSXHost::CreateModule(std::string& path)
	{
		std::cout << "Creating module " << path << std::endl;


		void* lib_handle = dlopen(path.c_str(), RTLD_LAZY | RTLD_GLOBAL);
		if (!lib_handle)
		{
			std::cerr << "Error load module: " << path << std::endl;
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

