/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */
#include <iostream>
#include <signal.h>
#include "javascript_module.h"

namespace kroll
{
	KROLL_MODULE(JavascriptModule)

	JavascriptModule* JavascriptModule::instance = NULL;
	void JavascriptModule::Initialize()
	{
		JavascriptModule::instance = this;
		host->AddModuleProvider(this);
	}

	void JavascriptModule::Stop()
	{
		JavascriptModule::instance = NULL;
	}


	const static std::string js_suffix = "module.js";

	bool JavascriptModule::IsModule(std::string& path)
	{
		int plength = path.length();
		int slength = js_suffix.length();
		if (path.length() > js_suffix.length())
		{
			return (path.substr(plength - slength) == js_suffix);
		}
		else
		{
			return false;
		}
	}

	Module* JavascriptModule::CreateModule(std::string& path)
	{
		JavascriptModuleInstance* instance =
			new JavascriptModuleInstance(this->host, path);
		return instance;
	}

	void JavascriptModule::Test()
	{
		JavascriptUnitTestSuite suite;
		suite.Run(host);
	}
}
