/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */
#include <iostream>
#include <signal.h>
#include "javascriptmodule.h"
#include "javascripttest.h"
	
namespace kroll
{
	KROLL_MODULE(JavascriptModule)

	JavascriptModule* JavascriptModule::instance = NULL;

	void JavascriptModule::Initialize()
	{
		KR_DUMP_LOCATION
		
		JavascriptModule::instance = this;

		host->AddModuleProvider(this);
	}

	void JavascriptModule::Destroy()
	{
		KR_DUMP_LOCATION
		
		// FIXME - unregister / unbind?
		JavascriptModule::instance = NULL;
	}


	const static std::string js_suffix = "module.js";

	bool JavascriptModule::IsModule(std::string& path)
	{
		return (path.substr(path.length()-js_suffix.length()) == js_suffix);
	}

	Module* JavascriptModule::CreateModule(std::string& path)
	{
		return NULL;
	}

	void JavascriptModule::Test()
	{
		JavascriptUnitTestSuite suite;
		suite.Run(host);
	}
}
