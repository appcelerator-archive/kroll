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

		ruby_init();
		ruby_init_loadpath();
		ruby_incpush(this->GetPath());

		RubyUtils::InitializeDefaultBindings(host);

		host->AddModuleProvider(this);
	}

	void RubyModule::Stop()
	{
		KR_DUMP_LOCATION

		ruby_cleanup(0);

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
		std::cout << "Create ruby module: " << path << std::endl;
		
		rb_load_file(path.c_str());
		ruby_exec();

//		ruby_cleanup();  <-- at some point we need to call?

		std::cout << "return new RubyModuleInstance " << path << std::endl;
		return new RubyModuleInstance(host, path);
	}

	void RubyModule::Test()
	{
		RubyUnitTestSuite suite;
		suite.Run(host);
	}
}
