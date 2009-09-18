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
#endif

#include <iostream>
#include <sstream>
#include <cstring>

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

	std::string Basename(std::string path)
	{
		size_t pos = path.find_last_of(KR_PATH_SEP_CHAR);
		if (pos == std::string::npos)
			return path;
		else
			return path.substr(pos+1);
	}

	bool CreateDirectory(std::string &dir, bool recursive)
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


	std::string GetDirectory(std::string &file)
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

	std::string Join(const char* inpart, ...)
	{
		va_list ap;
		va_start(ap, inpart);
		std::vector<std::string> parts;
		while (inpart != NULL)
		{
			if (strcmp(inpart, ""))
				parts.push_back(inpart);
			inpart = va_arg(ap, const char*);
		}
		va_end(ap);

		std::string filepath;
		std::vector<std::string>::iterator iter = parts.begin();
		while (iter != parts.end())
		{
			std::string part(*iter);
			bool first = (iter == parts.begin());
			bool last = (iter == parts.end()-1);
			iter++;

			part = Trim(part);
			if (part[part.size()-1] == KR_PATH_SEP_CHAR)
				part = part.erase(part.size() - 1, 1);
			if (!first && part[0] == KR_PATH_SEP_CHAR)
				part = part.erase(0, 1);
			filepath += part;

			if (!last)
				filepath += KR_PATH_SEP;
		}
#ifdef OS_OSX
		NSString *s = [[NSString stringWithUTF8String:filepath.c_str()] stringByExpandingTildeInPath];
		NSString *p = [s stringByStandardizingPath];
		@try
		{
			filepath = [p fileSystemRepresentation];
		}
		@catch (NSException *ex)
		{
			const char *reason = [[ex reason] UTF8String];
			printf("[Titanium.FileUtils] [Error] Error in Join: %s, '%s'\n", reason, filepath.c_str());
			return filepath;
		}
#endif
		return filepath;
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

	void Tokenize(const std::string& str,
		std::vector<std::string>& tokens, const std::string delimeters,
		bool skip_if_found)
	{
		std::string::size_type lastPos = str.find_first_not_of(delimeters,0);
		std::string::size_type pos = str.find_first_of(delimeters,lastPos);
		while (std::string::npos!=pos || std::string::npos!=lastPos)
		{
			std::string token(str.substr(lastPos,pos-lastPos));
			bool found = false;
			if (skip_if_found)
			{
				std::vector<std::string>::iterator i = tokens.begin();
				while(i!=tokens.end())
				{
					std::string entry(*i++);
					if (entry == token)
					{
						found = true;
						break;
					}
				}
			}
			if (!found)
			{
				tokens.push_back(token);
			}
			lastPos = str.find_first_not_of(delimeters,pos);
			pos = str.find_first_of(delimeters,lastPos);
		}
	}

	std::string Trim(std::string str)
	{
		std::string c(str);
		while (1)
		{
			size_t pos = c.rfind(" ");
			if (pos == std::string::npos || pos!=c.length()-1)
			{
				break;
			}
			c = c.substr(0,pos);
		}
		while(1)
		{
			size_t pos = c.find(" ");
			if (pos != 0)
			{
				break;
			}
			c = c.substr(1);
		}
		return c;
	}
}
}
