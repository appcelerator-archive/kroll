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
	PythonModuleInstance::PythonModuleInstance(Host *host, std::string path) :
		Module(host, path.c_str(), path.c_str(), "0.1"), path(path)
	{
	}

	PythonModuleInstance::~PythonModuleInstance()
	{
	}

	const char* PythonModuleInstance::GetName() 
	{ 
		return path.c_str(); 
	}

	void PythonModuleInstance::Initialize () 
	{
	}

	void PythonModuleInstance::Destroy () 
	{
	}
}

