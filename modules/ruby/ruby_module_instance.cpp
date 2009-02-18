/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */
#include "ruby_module_instance.h"

namespace kroll
{
	RubyModuleInstance::RubyModuleInstance(Host *host, std::string path) :
		Module(host, path), path(path)
	{
	}
	RubyModuleInstance::~RubyModuleInstance()
	{
	}
	const char* RubyModuleInstance::GetName() 
	{ 
		return path.c_str(); 
	}
	void RubyModuleInstance::Initialize () 
	{
	}
	void RubyModuleInstance::Destroy () 
	{
	}
}

