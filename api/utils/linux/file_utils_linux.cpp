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

using kroll::FileUtils;

std::string FileUtils::GetUserRuntimeHomeDirectory()
{
	std::string pname = PRODUCT_NAME;
	std::transform(pname.begin(), pname.end(), pname.begin(), tolower);
	pname = std::string(".") + pname;

	passwd *user = getpwuid(getuid());
	return Join(user->pw_dir, pname, NULL)
}

std::string FileUtils::GetSystemRuntimeHomeDirectory()
{
	// Try to be a little smart about where the system runtime home
	// is. If we can't find it, just give up and use the /opt location
	string path = Join("/opt", PRODUCT_NAME, NULL);
	if (!::IsDirectory(path))
		path = Join("/usr/local/lib", PRODUCT_NAME, NULL);
	if (!::IsDirectory(path))
		path = Join("/usr/lib", PRODUCT_NAME, NULL);
	if (!::IsDirectory(path))
		path = Join("/opt", PRODUCT_NAME, NULL);
	return path;
}
