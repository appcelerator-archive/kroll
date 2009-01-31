/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */

#ifndef _KR_ARG_UTILS_H_
#define _KR_ARG_UTILS_H_

#include <vector>
#include <string>
#include <map>

namespace kroll
{

	/*
		Class: ArgUtils
	*/
	class KROLL_API ArgUtils
	{

	public:
		/**
		 * Function: VerifyArgs
		 *   Verify an argument list given a collection of argument
		 *   string formats.
		 *
		 * Params:
		 *   args - The argument list
		 *   ... - A number of const char* arguments which specify
		 *         type of a paramter in the argument list. Some examples
		 *         include "s" for a string, "so" for a string or an object,
		 *         or "?so" for an optional string or object. The first
		 *         optional argument will cause the remaining arguments to
		 *         be optional.
		 *
		 *         Supported types:
		 *           s: string
		 *           i: int
		 *           d: double
		 *           n: number
		 *           o: object
		 *           m: method
		 *           l: list
		 *           0: null
		 * Returns:
		 *   True if the argument list is valid, false otherwise
		 */
		static bool VerifyArgs(const ValueList& args, ...);

	private:
		static inline bool VerifyArg(SharedValue arg, const char* type_string);

	};

}

#endif
