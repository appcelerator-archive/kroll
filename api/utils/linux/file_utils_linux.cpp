/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */
#include "../utils.h"
#include <cstdarg>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <linux/if.h>
#include <sys/utsname.h>
#include <libgen.h>

using std::string;

namespace UTILS_NS
{
	std::string FileUtils::GetUserRuntimeHomeDirectory()
	{
		string pname = PRODUCT_NAME;
		std::transform(pname.begin(), pname.end(), pname.begin(), tolower);
		pname = std::string(".") + pname;
	
		passwd *user = getpwuid(getuid());
		return Join(user->pw_dir, pname.c_str(), NULL);
	}

	std::string FileUtils::GetSystemRuntimeHomeDirectory()
	{
		// Try to be a little smart about where the system runtime home
		// is. If we can't find it, just give up and use the /opt location
		string pname = PRODUCT_NAME;
		std::transform(pname.begin(), pname.end(), pname.begin(), tolower);
		string path = Join("/opt", pname.c_str(), NULL);
		if (!IsDirectory(path))
			path = Join("/usr/local/lib", pname.c_str(), NULL);
		if (!IsDirectory(path))
			path = Join("/usr/lib", pname.c_str(), NULL);
		if (!IsDirectory(path))
			path = Join("/opt", pname.c_str(), NULL);
		return path;
	}

	std::string FileUtils::GetUsername()
	{
		char* loginName = getlogin();
		if (loginName != NULL)
		{
			return loginName;
		}
		else if (EnvironmentUtils::Has("USER"))
		{
			return EnvironmentUtils::Get("USER");
		}
		else
		{
			return "unknown";
		}
	}
}
