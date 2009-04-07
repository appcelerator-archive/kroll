/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */
#ifndef _KR_DATA_UTILS_H_
#define _KR_DATA_UTILS_H_
#include <string>
namespace kroll
{
	class KROLL_API DataUtils
	{
		public:
		/*
		 * @returns the hexidecimal MD5 hash of a string
		 */
		static std::string HexMD5(std::string);
	};
}
#endif
