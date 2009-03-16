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
		RubyModule::instance_ = this;

		ruby_init();
		ruby_init_loadpath();
		ruby_incpush(this->GetPath());
		this->InitializeBinding();

		host->AddModuleProvider(this);
	}

	void RubyModule::Stop()
	{
		SharedKObject global = this->host->GetGlobalObject();
		global->Set("Ruby", Value::Undefined);
		this->binding->Set("evaluate", Value::Undefined);
		this->binding = NULL;
		RubyModule::instance_ = NULL;

		ruby_cleanup(0);
	}

	void RubyModule::InitializeBinding()
	{
		// Expose the Ruby evaluator into Kroll
		SharedKObject global = this->host->GetGlobalObject();
		this->binding = new StaticBoundObject();
		global->Set("Ruby", Value::NewObject(binding));
		SharedKMethod evaluator = new RubyEvaluator();
		binding->Set("evaluate", Value::NewMethod(evaluator));

		// Bind the API global constant
		VALUE ruby_api_val = RubyUtils::KObjectToRubyValue(Value::NewObject(global));
		rb_define_global_const(PRODUCT_NAME, ruby_api_val);
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
