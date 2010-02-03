/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */
#include <iostream>
#include <signal.h>
#include "javascript_module.h"
#include <Poco/Path.h>

namespace kroll
{
	KROLL_MODULE(JavascriptModule, STRING(MODULE_NAME), STRING(MODULE_VERSION));

	JavascriptModule* JavascriptModule::instance = NULL;
	void JavascriptModule::Initialize()
	{
		JavascriptModule::instance = this;
		host->AddModuleProvider(this);
		
		KObjectRef global(Host::GetInstance()->GetGlobalObject());
		JavascriptMethods::Bind(global);
	}

	void JavascriptModule::Stop()
	{
		JavascriptModule::instance = NULL;
	}

	const static std::string jsSuffix = "module.js";
	bool JavascriptModule::IsModule(std::string& path)
	{
		int plength = path.length();
		int slength = jsSuffix.length();
		if (path.length() > jsSuffix.length())
		{
			return (path.substr(plength - slength) == jsSuffix);
		}
		else
		{
			return false;
		}
	}

	Module* JavascriptModule::CreateModule(std::string& path)
	{
		Poco::Path p(path);
		std::string basename = p.getBaseName();
		std::string name = basename.substr(0,basename.length()-jsSuffix.length()+3);
		std::string moduledir = path.substr(0,path.length()-basename.length()-3);

		Logger *logger = Logger::Get("Javascript");
		logger->Info("Loading JS path=%s", path.c_str());

		JavascriptModuleInstance* instance = new JavascriptModuleInstance(this->host, path, moduledir, name);
		return instance;
	}
}
