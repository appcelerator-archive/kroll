/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */

#include "utils.h"
#include <cstdlib>

#ifdef OS_WIN32
#include <windows.h>
#elif OS_OSX
#include <crt_externs.h>
#define environ (*_NSGetEnviron())
#else
extern char** environ;
#endif

namespace UTILS_NS
{
	bool EnvironmentUtils::Has(std::string name)
	{
#ifdef OS_WIN32
		DWORD len = GetEnvironmentVariableA(name.c_str(), 0, 0);
		return len > 0;
#else
		return getenv(name.c_str()) != 0;
#endif
	}

	std::string EnvironmentUtils::Get(std::string name)
	{
#ifdef OS_WIN32
		int MAX_LENGTH = 1024;
		char* buffer = new char[MAX_LENGTH];
		DWORD len = GetEnvironmentVariableA(name.c_str(), buffer, MAX_LENGTH - 1);
		if(len > 0)
		{
			std::string result(buffer);
			delete [] buffer;
			return result;
		}
		else
		{
			delete [] buffer;
			return std::string();
		}
#else
		const char* val = getenv(name.c_str());
		if (val)
			return std::string(val);
		else
			return std::string();
#endif
	}

	void EnvironmentUtils::Set(std::string name, std::string value)
	{
#ifdef OS_WIN32
		if (SetEnvironmentVariableA(name.c_str(), value.c_str()) == 0)
		{
			throw std::string("Cannot set environment variable: ") + name;
		}
#else
		if (setenv(name.c_str(), value.c_str(), 1))
		{
			throw std::string("Cannot set environment variable: ") + name;
		}
#endif
	}

	void EnvironmentUtils::Unset(std::string name)
	{
#ifdef OS_WIN32
		SetEnvironmentVariableA(name.c_str(), NULL);
#else
		unsetenv(name.c_str());
#endif
	}

	std::map<std::string, std::string> EnvironmentUtils::GetEnvironment()
	{
		std::map<std::string, std::string> environment;
#ifdef OS_WIN32
		LPTCH env = GetEnvironmentStrings();
		while (env[0] != '\0')
		{
			std::string entry = (char*)env;
			std::string key = entry.substr(0, entry.find("="));
			std::string val = entry.substr(entry.find("=")+1);
			environment[key] = val;
			env += entry.size() + 1;
		}
#else
		char** current = environ;
		while (*current)
		{
			std::string entry = *current;
			std::string key = entry.substr(0, entry.find("="));
			std::string val = entry.substr(entry.find("=")+1);
			environment[key] = val;
			current++;
		}
#endif
		return environment;
	}
}
