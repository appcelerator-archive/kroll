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
}
