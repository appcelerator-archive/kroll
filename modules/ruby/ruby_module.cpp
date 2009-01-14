/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */
#include "rubymodule.h"
#include "rubybinding.h"
#include "rubytypes.h"
#include <iostream>
#include <signal.h>

namespace kroll
{
	KROLL_MODULE(RubyModule)

	RubyModule* RubyModule::instance_ = NULL;

	void RubyModule::Initialize()
	{
		RubyModule::instance_ = this;

		ruby_init();
		ruby_init_loadpath();
		BoundObjectWrapper_Init();

		host->AddModuleProvider(this);
	}

	std::string ruby_suffix = "module.rb";

	bool RubyModule::IsModule(std::string& path)
	{
		std::cout << "RubyModule::IsModule ? " << path << std::endl;

		return (path.substr(path.length()-ruby_suffix.length()) == ruby_suffix);
	}

	/*VALUE ruby_exec_wrap(VALUE arg)
	{
		return INT2NUM(ruby_exec());
	}

	int ruby_exec_protect()
	{
		int error;
		VALUE response = rb_protect(ruby_exec_wrap, 0, &error);
		if (error) {
			std::cerr << "Error in script: " << error << std::endl;
		}
		return NUM2INT(response);
	}*/

	Module* RubyModule::CreateModule(std::string& path)
	{
		ruby_script(path.c_str());
		rb_load_file(path.c_str());
		//signal(SIGSEGV, ruby_signal);

		rb_gc_disable();
		status = ruby_exec();

		RubyModuleInstance *instance = new RubyModuleInstance(host, path);
		return instance;
	}

	void RubyModule::Destroy()
	{
		ruby_cleanup(status);
		RubyModule::instance_ = NULL;
	}
}
