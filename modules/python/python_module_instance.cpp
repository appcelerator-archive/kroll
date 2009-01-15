/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */
#include "python_module_instance.h"

namespace kroll
{
	PythonModuleInstance::PythonModuleInstance(Host *host, std::string path) :
		Module(host,FileUtils::GetDirectory(path)), path(path)
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

