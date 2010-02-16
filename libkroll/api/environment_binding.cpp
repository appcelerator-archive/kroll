/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */

#include "environment_binding.h"

namespace kroll
{
	KValueRef EnvironmentBinding::Get(const char *name)
	{
		return Value::NewString(EnvironmentUtils::Get(name));
	}
	
	SharedStringList EnvironmentBinding::GetPropertyNames()
	{
		std::map<std::string, std::string> env = EnvironmentUtils::GetEnvironment();
		SharedStringList keys = new StringList();
		
		std::map<std::string, std::string>::iterator iter = env.begin();
		for (; iter != env.end(); iter++)
		{
			keys->push_back(new std::string(iter->first));
		}
		return keys;
	}
	
	void EnvironmentBinding::Set(const char *name, KValueRef value)
	{
		if (value->IsString())
		{
			EnvironmentUtils::Set(name, value->ToString());
		}
	}
	
	SharedString EnvironmentBinding::DisplayString(int levels)
	{
		std::map<std::string, std::string> env = EnvironmentUtils::GetEnvironment();
		std::map<std::string, std::string>::iterator iter = env.begin();
		SharedString str = new std::string();
		
		for (; iter != env.end(); iter++)
		{
			(*str) += iter->first + "=" + iter->second + ",";
		}
		return str;
	}
}
