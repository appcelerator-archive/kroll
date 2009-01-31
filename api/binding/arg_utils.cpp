/*
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */

#include "../kroll.h"

namespace kroll
{

	bool ArgUtils::VerifyArgs(const ValueList& args, ...)
	{
		va_list list;
		const char* type_string;
		size_t i = 0;

		va_start(list, args);
		type_string = va_arg(list, const char *);
		bool optional = false;
		while (type_string != NULL)
		{
			// The first time we see the optional
			// parameter we stay in optional mode
			// until the end of the session.
			if (*type_string == '?')
			{
				optional = true;
				*type_string++;
			}

			// Not enough args given, but we're in
			// optional mode.
			if (args.size() < i + 1 && optional)
			{
				va_end(list);
				return true;
			}

			// Not enough args given.
			if (args.size() < i + 1)
			{
				va_end(list);
				return false;
			}

			// Arg doesn't conform to arg string
			if (!VerifyArg(args.at(i), type_string))
			{
				va_end(list);
				return false;
			}

			i++;
			type_string = va_arg(list, const char *); // Test next
		}

		va_end(list);
		return true;
	}


	inline bool ArgUtils::VerifyArg(SharedValue arg, const char* type_string)
	{
		while (*type_string != '\0')
		{
		// Check if type of value matches current character.
		if ((*type_string == 's' && arg->IsString())
		 || (*type_string == 'i' && arg->IsInt())
		 || (*type_string == 'd' && arg->IsDouble())
		 || (*type_string == 'n' && arg->IsNumber())
		 || (*type_string == 'o' && arg->IsObject())
		 || (*type_string == 'l' && arg->IsList())
		 || (*type_string == 'm' && arg->IsMethod())
		 || (*type_string == '0' && arg->IsNull()))
			return true;
		else
			*type_string++;
		}
		return false;
	}

}

