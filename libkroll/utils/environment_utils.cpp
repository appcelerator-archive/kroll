/** * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */

#include "utils.h"
#include <cstdlib>

#ifdef OS_WIN32
// See http://msdn.microsoft.com/en-us/library/ms686206(VS.85).aspx
#define MAX_ENV_VALUE_SIZE 32767 
#define REASONABLE_MAX_ENV_VALUE_SIZE 1024
#include <windows.h>
#elif OS_OSX
#include <crt_externs.h>
#define environ (*_NSGetEnviron())
#else
extern char** environ;
#endif

namespace UTILS_NS
{
namespace EnvironmentUtils
{
	bool Has(std::string name)
	{
#ifdef OS_WIN32
		std::wstring wideName = UTF8ToWide(name);
		DWORD len = GetEnvironmentVariableW(wideName.c_str(), 0, 0);
		return len > 0;
#else
		return getenv(name.c_str()) != 0;
#endif
	}

	std::string Get(std::string name)
	{
#ifdef OS_WIN32
		// Allocate a small buffer first, before taking the plunge
		// and allocating the maximum size. Hopefully this will prevent
		// expensive allocations.
		wchar_t* buffer = new wchar_t[REASONABLE_MAX_ENV_VALUE_SIZE];
		std::wstring wideName(UTF8ToWide(name));

		DWORD size = GetEnvironmentVariableW(wideName.c_str(), buffer, REASONABLE_MAX_ENV_VALUE_SIZE - 1);
		if (size > REASONABLE_MAX_ENV_VALUE_SIZE)
		{
			// This is a humongous environment variable value, so we need to allocate the
			// max size and grab it that way.
			delete [] buffer;
			buffer = new wchar_t[MAX_ENV_VALUE_SIZE];
			size = GetEnvironmentVariableW(wideName.c_str(), buffer, MAX_ENV_VALUE_SIZE - 1);
		}

		if (size > 0)
		{
			buffer[size] = '\0';
			std::string result(WideToUTF8(buffer));
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

	void Set(std::string name, std::string value)
	{
#ifdef OS_WIN32
		std::wstring wideName(UTF8ToWide(name));
		std::wstring wideValue(UTF8ToWide(value));
		if (SetEnvironmentVariableW(wideName.c_str(), wideValue.c_str()) == 0)
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

	void Unset(std::string name)
	{
#ifdef OS_WIN32
		std::wstring wideName(UTF8ToWide(name));
		SetEnvironmentVariableW(wideName.c_str(), NULL);
#else
		unsetenv(name.c_str());
#endif
	}

#if defined(KROLL_API_EXPORT) || defined(_KROLL_H_)
	std::map<std::string, std::string> GetEnvironment()
	{
		std::map<std::string, std::string> environment;
#ifdef OS_WIN32
		LPWCH env = GetEnvironmentStringsW();
		while (env[0] != '\0')
		{
			std::wstring entryW(env);
			std::string entry(WideToUTF8(entryW));
			std::string key(entry.substr(0, entry.find("=")));
			std::string val(entry.substr(entry.find("=")+1));
			environment[key] = val;
			env += entry.size() + 1;
		}
#else
		char** current = environ;
		while (*current)
		{
			std::string entry(*current);
			std::string key(entry.substr(0, entry.find("=")));
			std::string val(entry.substr(entry.find("=")+1));
			environment[key] = val;
			current++;
		}
#endif
		return environment;
	}
#endif
}
}
