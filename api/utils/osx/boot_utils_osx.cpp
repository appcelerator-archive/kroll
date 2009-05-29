/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */
#include "../utils.h"

namespace UTILS_NS
{
	vector<string>& BootUtils::GetComponentSearchPaths()
	{
		static bool initialized = false;
		if (!initialized)
		{
			// Allow the user to force an override to the runtime home by setting the
			// appropriate environment variable -- this will be the first path searched
			if (EnvironmentUtils::Has("KR_RUNTIME_HOME"))
			{
				componentSearchPaths.push_back(EnvironmentUtils::Get("KR_RUNTIME_HOME"));
			}

			componentSearchPaths.push_back(FileUtils::GetUserRuntimeHomeDirectory());
			componentSearchPaths.push_back(FileUtils::GetSystemRuntimeHomeDirectory());
			initialized = true;
		}
		return componentSearchPaths;
	}
}
