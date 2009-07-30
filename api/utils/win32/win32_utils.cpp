/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */
#include "../utils.h"

namespace UTILS_NS
{
	std::string Win32Utils::QuickFormatMessage(DWORD errorCode, DWORD flags)
	{
		std::string result;
		LPSTR errorText = NULL;
		FormatMessageA(
			flags,
			NULL, // unused with FORMAT_MESSAGE_FROM_SYSTEM
			errorCode,
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
			(LPSTR) &errorText,
			0, // minimum size for output buffer
			NULL);

		if (errorText)
		{
			result = errorText;
	   		LocalFree(errorText);
		}
		return result;
	}

	std::wstring UTF8ToWide(std::string& in)
	{
		return UTF8ToWide(in.c_str());
	}

	std::wstring UTF8ToWide(const char* in)
	{
		int size = strlen(in);
		if (size == 0)
			return L"";

	 	wchar_t* buffer = new wchar_t[size + 1];
	 	buffer[size] = '\0';
	
	 	MultiByteToWideChar(CP_UTF8, 0, in, -1, buffer, size);
	 	std::wstring out = buffer;
	 	delete [] buffer;
	 	return out; 
	 
	}

	std::string WideToUTF8(std::wstring& in)
	{
		return WideToUTF8(in.c_str());
	}

	std::string WideToUTF8(const wchar_t* in)
	{
		int size = wcslen(in);
		if (size == 0)
			return "";

		int bufferSize = (4 * size) + 1;
		char* buffer = new char[4 * size + 1];
		buffer[4 * size] = '\0';
	
		WideCharToMultiByte(CP_UTF8, 0, in, -1, buffer, bufferSize - 1, NULL, NULL);
		std::string out = buffer;
		delete [] buffer;
		return out; 
	}
}
