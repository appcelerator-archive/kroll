/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */
#include "python_module_instance.h"

namespace kroll
{
	// TODO: Implement real method metadata and lifecycle events for
	// scripting language-based modules
	PythonModuleInstance::PythonModuleInstance(Host *host, std::string path, std::string dir, std::string name) :
		Module(host, dir.c_str(), name.c_str(), "0.1"),
		path(path)
	{
	}

	PythonModuleInstance::~PythonModuleInstance()
	{
	}

	void PythonModuleInstance::Initialize () 
	{
	}

	void PythonModuleInstance::Destroy () 
	{
	}
}

