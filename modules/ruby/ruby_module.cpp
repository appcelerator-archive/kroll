/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */
#include <signal.h>
#include "ruby_module.h"

namespace kroll
{
	KROLL_MODULE(RubyModule)

	RubyModule* RubyModule::instance_ = NULL;

	void RubyModule::Initialize()
	{
		KR_DUMP_LOCATION

		RubyModule::instance_ = this;

		RubyUtils::InitializeDefaultBindings(host);

		host->AddModuleProvider(this);
	}

	void RubyModule::Destroy()
	{
		KR_DUMP_LOCATION

		// FIXME - unregister / unbind?
		RubyModule::instance_ = NULL;
	}


	const static std::string ruby_suffix = "module.rb";

	bool RubyModule::IsModule(std::string& path)
	{
		return (path.substr(path.length()-ruby_suffix.length()) == ruby_suffix);
	}

	Module* RubyModule::CreateModule(std::string& path)
	{
		char* path2 = (char*)malloc(sizeof(char)*path.length()+1);
		size_t length = path.copy(path2, path.length(), 0);
		path2[length] = '\0';

		std::cout << "Create module: " << path2 << std::endl;

		// FILE *file = fopen(path2, "r");
		// printf("got ruby file: %d\n", (int) file);
		// 
		// //FIXME - we need to create a separate version of scope stuff
		// 
		// // right now ruby is crashing in win32, need to investigate
		// PyRun_SimpleFile(file,path2);
		// std::cout << "PyRan simple file" << std::endl;

		std::string path3(path2);
		free(path2);

		std::cout << "return new RubyModuleInstance " << path3 << std::endl;
		return new RubyModuleInstance(host, path3);
	}

	void RubyModule::Test()
	{
		RubyUnitTestSuite suite;
		suite.Run(host);
	}
}
