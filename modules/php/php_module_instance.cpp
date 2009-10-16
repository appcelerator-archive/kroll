/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */
#include "php_module_instance.h"

namespace kroll
{
	PHPModuleInstance::PHPModuleInstance(Host* host, std::string path, std::string dir, std::string name) :
		Module(host, dir.c_str(), name.c_str(), "0.1"),
		path(path)
	{
	}

	PHPModuleInstance::~PHPModuleInstance()
	{
	}

	void PHPModuleInstance::Initialize ()
	{
	}

	void PHPModuleInstance::Destroy ()
	{
	}
}

