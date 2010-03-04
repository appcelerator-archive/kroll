/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */
#include "utils.h"

#ifdef OS_OSX
#include <Cocoa/Cocoa.h>
#include <IOKit/IOKitLib.h>
#include <IOKit/network/IOEthernetInterface.h>
#include <IOKit/network/IONetworkInterface.h>
#include <IOKit/network/IOEthernetController.h>
#include <sys/utsname.h>
#include <libgen.h>
#elif defined(OS_LINUX)
#include <cstdarg>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/utsname.h>
#include <libgen.h>
#elif defined(OS_WIN32)
#include <cctype>
#include <cwchar>
#endif


#include <iostream>
#include <sstream>
#include <cstring>
#include <locale>
#include <cwctype>

namespace UTILS_NS
{
namespace FileUtils
{
	std::string GetApplicationDataDirectory(std::string &appid)
	{
		std::string dir(GetUserRuntimeHomeDirectory());
		dir = FileUtils::Join(dir.c_str(), "appdata", appid.c_str(), NULL);
		CreateDirectory(dir, true);

		return dir;
	}

	std::string Basename(const std::string& path)
	{
		size_t pos = path.find_last_of(KR_PATH_SEP_CHAR);
		if (pos == std::string::npos)
			return path;
		else
			return path.substr(pos+1);
	}

	bool CreateDirectory(const std::string& dir, bool recursive)
	{
		if (IsDirectory(dir))
		{
			return true;
		}
		
		string parent(Dirname(dir));
		if (recursive && parent.size() > 0 && !IsDirectory(parent))
		{
			if (!CreateDirectory(parent, true))
				return false;
		}

		return CreateDirectoryImpl(dir);
	}


	std::string GetDirectory(const std::string& file)
	{
		size_t pos = file.find_last_of(KR_PATH_SEP);
		if (pos == std::string::npos)
		{
			pos = file.find_last_of(KR_PATH_SEP_OTHER);
			if (pos == std::string::npos)
			{
				return "."KR_PATH_SEP;
			}
		}
#ifdef OS_OSX
		NSString *s = [[NSString stringWithCString:file.substr(0,pos).c_str() encoding:NSUTF8StringEncoding] stringByExpandingTildeInPath];
		return [s fileSystemRepresentation];
#else
		return file.substr(0, pos);
#endif
	}

	template <class StringType, class CharType> static StringType JoinTemplate(
		const CharType separator, const CharType* nextPart, va_list args)
	{
		bool first = true;
		StringType filePath;
		while (nextPart)
		{
			StringType part(Trim(nextPart));
			nextPart = va_arg(args, const CharType*);
			if (part.empty()) // Skip empty bits.
				continue;

			// Erase any existing path separators.
			if (part[part.size()-1] == separator)
				part = part.erase(part.size() - 1, 1);

			// If this is the first component, the first character
			// may be '/' and this may be an absolute path, so don't
			// erase.
			if (!first && part[0] == separator)
				part = part.erase(0, 1);

			if (!first)
				filePath.push_back(separator);
			else
				first = false;
			filePath.append(part); // Append!
		}

		return filePath;
	}

	std::string Join(const char* firstPart, ...)
	{
		va_list args;
		va_start(args, firstPart);
		std::string result = JoinTemplate<std::string, char>(
			KR_PATH_SEP_CHAR, firstPart, args);
		va_end(args);

#ifdef OS_OSX
		@try
		{
			result = [[[[NSString stringWithUTF8String:result.c_str()]
				stringByExpandingTildeInPath]
				stringByStandardizingPath]
				fileSystemRepresentation];
		}
		@catch (NSException* ex)
		{
			printf("[Titanium.FileUtils] [Error] Error joining %s: %s",
				result.c_str(), [[ex reason] UTF8String]);
		}
#endif

		return result;
	}

	std::wstring Join(const wchar_t* firstPart, ...)
	{
		va_list args;
		va_start(args, firstPart);
		std::wstring result = JoinTemplate<std::wstring, wchar_t>(
			WIDE_KR_PATH_SEP_CHAR, firstPart, args);
		va_end(args);
		return result;
	}

	std::string GetOSVersion()
	{
#ifdef OS_WIN32
		OSVERSIONINFO vi;
		vi.dwOSVersionInfoSize = sizeof(vi);
		if (GetVersionEx(&vi) == 0) return "?";

		std::ostringstream str;
		str << vi.dwMajorVersion << "." << vi.dwMinorVersion << " (Build " << (vi.dwBuildNumber & 0xFFFF);
		if (vi.szCSDVersion[0]) str << ": " << vi.szCSDVersion;
		str << ")";
		return str.str();
#elif OS_OSX || OS_LINUX
		struct utsname uts;
		uname(&uts);
		return uts.release;
#endif
	}

	std::string GetOSArchitecture()
	{
#ifdef OS_WIN32
		return std::string("win32");
#elif OS_OSX || OS_LINUX
		struct utsname uts;
		uname(&uts);
		return uts.machine;
#endif
	}

	template <class T> static void TokenizeTemplate(const T& haystack,
		std::vector<T>& tokens, const T& delimeters, bool skipDuplicates)
	{
		size_t lastPos = haystack.find_first_not_of(delimeters, 0);
		size_t pos = haystack.find_first_of(delimeters, lastPos);
		while (T::npos != pos || T::npos != lastPos)
		{
			T token(haystack.substr(lastPos, pos-lastPos));
			bool duplicate = false;

			if (skipDuplicates)
			{
				for (size_t i = 0; i < tokens.size(); i++)
				{
					if (tokens[i] == token)
					{
						duplicate = true;
						break;
					}
				}
			}

			if (!duplicate)
				tokens.push_back(token);

			lastPos = haystack.find_first_not_of(delimeters, pos);
			pos = haystack.find_first_of(delimeters, lastPos);
		}
	}

	void Tokenize(const std::string& haystack, std::vector<std::string>& tokens,
		const std::string& delimeters, bool skipDuplicates)
	{
		TokenizeTemplate<std::string>(haystack, tokens, delimeters, skipDuplicates);
	}

	void TokenizeWide(const std::wstring& haystack, std::vector<std::wstring>& tokens,
		const std::wstring& delimeters, bool skipDuplicates)
	{
		TokenizeTemplate<std::wstring>(haystack, tokens, delimeters, skipDuplicates);
	}

	template <class CharType> static bool IsSpace(CharType );

	template <> bool IsSpace(char c)
	{
		return isspace(c);
	}

	template <> bool IsSpace(wchar_t c)
	{
		return iswspace(c);
	}

	template <class StringType> static StringType TrimTemplate(StringType string)
	{
		if (string.empty())
			return string;

		size_t left = 0;
		size_t right = string.size() - 1;
		while (IsSpace(string[left]) && left < string.size() - 1)
			left++;

		while (IsSpace(string[right]) && right > left)
			right--;

		return string.substr(left, right - left + 1);
	}

	std::string Trim(std::string string)
	{
		return TrimTemplate<std::string>(string);
	}

	std::wstring Trim(std::wstring string)
	{
		return TrimTemplate<std::wstring>(string);
	}
}
}
