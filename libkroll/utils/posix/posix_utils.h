/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2009-2010 Appcelerator, Inc. All Rights Reserved.
 */
#ifndef _KR_POSIX_UTILS_H_
#define _KR_POSIX_UTILS_H_
#include <string>
namespace UTILS_NS
{
	KROLL_API std::wstring UTF8ToWide(const std::string& in);
	KROLL_API std::wstring UTF8ToWide(const char* in);
	KROLL_API std::string WideToUTF8(const std::wstring& in);
	KROLL_API std::string WideToUTF8(const wchar_t* in);
	KROLL_API std::string UTF8ToSystem(const std::string& in);
	KROLL_API std::string UTF8ToSystem(const char* in);
}
#endif
