/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */
#include "../utils.h"

#include <windows.h>
#include <shlobj.h>
#include <Iphlpapi.h>
#include <process.h>

using kroll::FileUtils;

std::string FileUtils::GetUserRuntimeHomeDirectory()
{
	char path[MAX_PATH];
	if (SHGetSpecialFolderPath(NULL, path, CSIDL_APPDATA, FALSE))
	{
		return Join(path, PRODUCT_NAME, NULL);
	}
	else
	{
		// Not good! What do we do in this case? I guess just use
		// the current directory. Perhaps this should be a temporary
		// directory?
		return "";
	}
}

std::string FileUtils::GetSystemRuntimeHomeDirectory()
{
	char path[MAX_PATH];
	if (SHGetSpecialFolderPath(NULL, path, CSIDL_COMMON_APPDATA, FALSE))
	{
		return Join(path, PRODUCT_NAME, NULL);
	}
	else
	{
		return GetUserRuntimeHomeDirectory();
	}
}
