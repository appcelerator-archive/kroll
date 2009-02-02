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
		 *   string formats. Note that VerifyArgs is not intended for
		 *   use in performance sensitive methods.
		 *
		 * Params:
		 *   args - The argument list
		 *   sig - A string containing argument characters separated by
		 *        commas. Some examples include "s" for a string, "so"
		 *        for a string or an object, or "?so" for an optional string
		 *        or object and "s,?so,l,m" for a complete signature string.
		 *        Note that the first optional argument will cause the
		 *        remaining arguments to be optional.
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
		static bool VerifyArgs(const ValueList& args, const char* sig);
		static void VerifyArgsException(const char *name, const ValueList& args, const char* sig);


	private:
		static inline std::vector<std::string>* ParseSigString(const char* sig);
		static SharedString GenerateSignature(const char* name, std::vector<std::string>* sig_vector);
		static bool VerifyArgsImpl(const ValueList& args, std::vector<std::string>* sig_vector);
		static inline bool VerifyArg(SharedValue arg, const char* t);

	};

}

#endif
