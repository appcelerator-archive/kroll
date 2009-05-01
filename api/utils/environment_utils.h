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

	class KROLL_API EnvironmentUtils
	{
		public:
		/*
		 * @returns whether or not the named environment variable exists.
		 */
		static bool Has(std::string name);

		/*
		 * @returns the given environment variable or empty string if it does
		 * not exist.
		 */
		static std::string Get(std::string name);

		/*
		 * Set an environment variable given a value and a name
		 */
		static void Set(std::string name, std::string value);

		/*
		 * Unset an environment variable given a name
		 */
		static void Unset(std::string name);
	};
}
#endif
