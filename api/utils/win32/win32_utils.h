/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */
#ifndef _KR_WIN32_UTILS_H_
#define _KR_WIN32_UTILS_H_
#include <string>
namespace UTILS_NS
{
	namespace Win32Utils
	{
		/*
		 * Easily get the result of a FormatMessage
		 * @param errorCode the error code to get the message for
		 * @returns a normal, all-American std::string for once
		 */
		KROLL_API std::string QuickFormatMessage(
			DWORD errorCode,
			DWORD flags=FORMAT_MESSAGE_FROM_SYSTEM |
				FORMAT_MESSAGE_ALLOCATE_BUFFER |
				FORMAT_MESSAGE_IGNORE_INSERTS);
	};

	namespace FileUtils
	{
		KROLL_API void CopyRecursive(std::string &dir, std::string &dest,
			std::string exclude = "");

		/* Some wchar versions of popular FileUtils functions */
		KROLL_API bool IsFile(std::wstring& path);
		KROLL_API std::string ReadFile(std::wstring& path);
	}

	KROLL_API std::wstring UTF8ToWide(std::string& in);
	KROLL_API std::wstring UTF8ToWide(const char* in);
	KROLL_API std::string WideToUTF8(std::wstring& in);
	KROLL_API std::string WideToUTF8(const wchar_t* in);
	KROLL_API std::wstring MBToWide(std::string& in, size_t size, UINT codePage);
	KROLL_API std::wstring MBToWide(const char* in, size_t size, UINT codePage);
}
#endif
