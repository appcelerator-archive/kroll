/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */
#ifndef _KR_DATA_UTILS_H_
#define _KR_DATA_UTILS_H_
#include <string>
namespace UTILS_NS
{
	namespace DataUtils
	{
		/*
		 * @returns the hexidecimal MD5 hash of a string
		 */
		KROLL_API std::string HexMD5(std::string);

		/**
		 * Generate a new UUID
		 * @returns a new UUID as a string
		 */
		KROLL_API std::string GenerateUUID();
	}
}
#endif
