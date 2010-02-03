/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */
#ifndef _KR_ENVIRONMENT_UTILS_H_
#define _KR_ENVIRONMENT_UTILS_H_
#include <string>

namespace UTILS_NS
{

	namespace EnvironmentUtils
	{
		/*
		 * @returns whether or not the named environment variable exists.
		 */
		KROLL_API bool Has(std::string name);

		/*
		 * @returns the given environment variable or empty string if it does
		 * not exist.
		 */
		KROLL_API std::string Get(std::string name);

		/*
		 * Set an environment variable given a value and a name
		 */
		KROLL_API void Set(std::string name, std::string value);

		/*
		 * Unset an environment variable given a name
		 */
		KROLL_API void Unset(std::string name);

#if defined(KROLL_API_EXPORT) || defined(_KROLL_H_)
		/*
		 * Get the environment
		 */
		KROLL_API std::map<std::string, std::string> GetEnvironment();
#endif
	};
}
#endif
